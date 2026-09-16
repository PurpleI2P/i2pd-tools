#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <inttypes.h>
#include <vector>
#include <string.h>
#include <memory>
#include "Blinding.h"
#include "I2PEndian.h"
#include "Crypto.h"
#include "Identity.h"
#include "Timestamp.h"
#include "common/key.hpp"

const int kMaxDays = 0xFFFF; // the number of keys in the batch is two bytes


static std::vector<uint8_t> createB33OfflineKeys (const i2p::data::PrivateKeys& keys,
	i2p::data::SigningKeyType transientSigType, int days)
{
	std::vector<uint8_t> buf;
	if (days < 1 || days > kMaxDays) return buf;
	auto identity = keys.GetPublic ();
	i2p::data::BlindedPublicKey blindedKey (identity);
	if (!blindedKey.IsValid ()) return buf;
	std::unique_ptr<i2p::crypto::Verifier> transientVerifier (i2p::data::IdentityEx::CreateVerifier (transientSigType));
	std::unique_ptr<i2p::crypto::Verifier> blindedVerifier (i2p::data::IdentityEx::CreateVerifier (blindedKey.GetBlindedSigType ()));
	if (!transientVerifier || !blindedVerifier) return buf;
	const size_t blindedSignatureLen = blindedVerifier->GetSignatureLen ();
	const size_t transientPublicKeyLen = transientVerifier->GetPublicKeyLen ();
	const size_t signedDataLen = i2p::data::OFFLINE_SIGNATURE_HEADER_LENGTH + transientPublicKeyLen;
	const size_t keyLen = signedDataLen + blindedSignatureLen + transientVerifier->GetPrivateKeyLen ();

	buf.resize (i2p::data::B33_OFFLINE_KEYS_HEADER_LENGTH + days*keyLen);
	size_t offset = 0;
	buf[offset] = i2p::data::B33_OFFLINE_KEYS_VERSION; offset++;
	memcpy (buf.data () + offset, identity->GetIdentHash (), 32); offset += 32;
	htobe16buf (buf.data () + offset, days); offset += 2;
	const uint64_t midnight = (i2p::util::GetSecondsSinceEpoch ()/i2p::data::SECONDS_PER_DAY)*i2p::data::SECONDS_PER_DAY;
	for (int i = 0; i < days; i++)
	{
		char date[9];
		i2p::util::GetDateString (midnight + i*i2p::data::SECONDS_PER_DAY, date);
		uint8_t * signedData = buf.data () + offset;
		htobe32buf (signedData, midnight + (i + 1)*i2p::data::SECONDS_PER_DAY); // expires at the end of that day
		htobe16buf (signedData + 4, transientSigType);
		uint8_t blindedPriv[32], blindedPub[32];
		if (!blindedKey.BlindPrivateKey (keys.GetSigningPrivateKey (), date, blindedPriv, blindedPub))
		{
			buf.clear ();
			return buf;
		}
		std::unique_ptr<i2p::crypto::Signer> blindedSigner (
			i2p::data::PrivateKeys::CreateSigner (blindedKey.GetBlindedSigType (), blindedPriv));
		if (!blindedSigner)
		{
			buf.clear ();
			return buf;
		}
		i2p::data::PrivateKeys::GenerateSigningKeyPair (transientSigType,
			signedData + signedDataLen + blindedSignatureLen, signedData + i2p::data::OFFLINE_SIGNATURE_HEADER_LENGTH);
		blindedSigner->Sign (signedData, signedDataLen, signedData + signedDataLen);
		memset (blindedPriv, 0, sizeof (blindedPriv)); // it would give the destination's key away
		offset += keyLen;
	}
	return buf;
}

static bool writeFile (const char * name, const void * data, std::size_t size)
{
	std::ofstream f (name, std::ofstream::binary | std::ofstream::out);
	f.write ((const char *)data, size);
	if (!f)
	{
		std::cout << "Can't write " << name << std::endl;
		return false;
	}
	return true;
}

int main (int argc, char * argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: b33offlinekeys <router keys file> <destination keys file> <days>" << std::endl;
		return -1;
	}
	i2p::crypto::InitCrypto (false);

	std::string fname(argv[2]);
	i2p::data::PrivateKeys keys;
	{
		std::vector<uint8_t> buff;
		std::ifstream inf;
		inf.open(fname);
		if (!inf.is_open()) {
			std::cout << "cannot open keys file " << fname << std::endl;
			return 2;
		}
		inf.seekg(0, std::ios::end);
		const std::size_t len = inf.tellg();
		inf.seekg(0, std::ios::beg);
		buff.resize(len);
		inf.read((char*)buff.data(), buff.size());
		if (!keys.FromBuffer(buff.data(), buff.size())) {
			std::cout << "bad keys file format" << std::endl;
			return 3;
		}
	}
	if (keys.IsOfflineSignature ())
	{
		std::cout << "the destination's own keys are required, " << fname << " holds offline keys" << std::endl;
		return 4;
	}
	// only a blindable destination has a b33 address at all
	if (!i2p::data::BlindedPublicKey (keys.GetPublic ()).IsValid ())
	{
		std::cout << fname << " is a " << SigTypeToName (keys.GetPublic ()->GetSigningKeyType ())
			<< " destination and has no b33 address; ED25519-SHA512 or RED25519-SHA512 is required" << std::endl;
		return 5;
	}

	int days = 365; // 1 year by default
	if (argc > 3)
		days = std::stoi (argv[3]);
	if (days < 1 || days > kMaxDays)
	{
		std::cout << "days must be 1.." << kMaxDays << std::endl;
		return 6;
	}

	// The outer layer of an encrypted LeaseSet is built on blinded keys, which are RedDSA
	const i2p::data::SigningKeyType type = i2p::data::SIGNING_KEY_TYPE_REDDSA_SHA512_ED25519;
	auto b33Keys = createB33OfflineKeys (keys, type, days);
	if (b33Keys.empty ())
	{
		std::cout << "Can't create b33 offline keys" << std::endl;
		return 7;
	}
	// the transient of the inner LeaseSet lasts exactly as long as the batch
	const uint32_t expires = (i2p::util::GetSecondsSinceEpoch ()/i2p::data::SECONDS_PER_DAY + days)*i2p::data::SECONDS_PER_DAY;
	auto onlineKeys = keys.CreateOfflineKeys (i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519, expires);
	std::vector<uint8_t> out(onlineKeys.GetFullLen () + b33Keys.size ());
	size_t l = onlineKeys.ToBuffer (out.data (), out.size ());
	if (!l)
	{
		std::cout << "Can't serialize online keys" << std::endl;
		return 8;
	}
	memcpy (out.data () + l, b33Keys.data (), b33Keys.size ());
	out.resize (l + b33Keys.size ());
	if (writeFile (argv[1], out.data (), out.size ()))
	{
		i2p::data::BlindedPublicKey blindedKey (keys.GetPublic ());
		std::cout << "Address " << blindedKey.ToB33 () << ".b32.i2p, " << days << " days" << std::endl
			<< "Give the router this file and keep the destination keys offline:" << std::endl
			<< "  keys = " << argv[1] << std::endl
			<< "  i2cp.leaseSetType = 5" << std::endl;
	}

	i2p::crypto::TerminateCrypto ();

	return 0;
}
