#include "Identity.h"
#include "I2PEndian.h"
#include "LeaseSet.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "common/key.hpp"


// version || ident hash || number of keys, then per day expires || transient signature type ||
// transient public key || signature by that day's blinded key || transient private key
static void printB33OfflineKeys (const i2p::data::PrivateKeys& keys, const i2p::data::BlindedPublicKey& blindedKey)
{
	const auto& batch = keys.GetB33OfflineKeys ();
	if (!batch.GetLen ()) return;
	std::unique_ptr<i2p::crypto::Verifier> blindedVerifier (
		i2p::data::IdentityEx::CreateVerifier (blindedKey.GetBlindedSigType ()));
	if (!blindedVerifier) return;
	const uint8_t * buf = batch.GetBuffer ();
	size_t offset = i2p::data::B33_OFFLINE_KEYS_HEADER_LENGTH - 2;
	uint16_t numKeys = bufbe16toh (buf + offset); offset += 2;
	std::string firstDate, lastDate;
	for (uint16_t i = 0; i < numKeys; i++)
	{
		std::unique_ptr<i2p::crypto::Verifier> transientVerifier (
			i2p::data::IdentityEx::CreateVerifier (bufbe16toh (buf + offset + 4)));
		if (!transientVerifier) return;
		char date[9];
		i2p::util::GetDateString (bufbe32toh (buf + offset) - i2p::data::SECONDS_PER_DAY, date);
		if (!i) firstDate = date;
		lastDate = date;
		offset += i2p::data::OFFLINE_SIGNATURE_HEADER_LENGTH + transientVerifier->GetPublicKeyLen () +
			blindedVerifier->GetSignatureLen () + transientVerifier->GetPrivateKeyLen ();
	}
	std::cout << "b33 offline keys: " << numKeys << " days, " << firstDate << " to " << lastDate << std::endl;
}

static int printHelp(const char * exe, int exitcode)
{
	std::cout << "usage: " << exe << " [-v] [-d] [-p] [-b] privatekey.dat" << std::endl;
	return exitcode;
}

std::string ConvertTime (time_t t)
{
	struct tm *tm = localtime(&t);
	char date[128];
	snprintf(date, sizeof(date), "%02d/%02d/%d %02d:%02d:%02d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return date;
}

int main(int argc, char * argv[])
{
	if(argc == 1) {
		return printHelp(argv[0], -1);
	}

	int opt;
	bool print_dest = false;
	bool print_private = false;
	bool print_blinded = false;
	bool verbose = false;
	while((opt = getopt(argc, argv, "hvdpb")) != -1) {
		switch(opt){
		case 'h':
			return printHelp(argv[0], 0);
		case 'v':
			verbose = true;
			break;
		case 'd':
			print_dest = true;
			break;
		case 'p':
			print_private = true;
			break;
		case 'b':
			print_blinded = true;
			break;
		default:
			return printHelp(argv[0], -1);
		}
	}

	std::string fname(argv[optind]);
	i2p::data::PrivateKeys keys;
	std::ifstream s(fname, std::ifstream::binary);

	if (!s.is_open()) {
		std::cout << "cannot open private key file " << fname << std::endl;
		return 2;
	}

	s.seekg(0, std::ios::end);
	size_t len = s.tellg();
	s.seekg(0, std::ios::beg);
	uint8_t * buf = new uint8_t[len];
	s.read((char*)buf, len);

	if (!keys.FromBuffer(buf, len)) {
		std::cout << "bad key file format" << std::endl;
		return 3;
	}

	auto dest = keys.GetPublic();
	if(!dest) {
		std::cout << "failed to extract public key" << std::endl;
		return 3;
	}

	const auto & ident = dest->GetIdentHash();
	if (verbose) {
		std::cout << "Destination: " << dest->ToBase64() << std::endl;
		std::cout << "Destination Hash: " << ident.ToBase64() << std::endl;
		std::cout << "B32 Address: " << ident.ToBase32() << ".b32.i2p" << std::endl;
		std::cout << "Signature Type: " << SigTypeToName(dest->GetSigningKeyType()) << std::endl;
		std::cout << "Encryption Type: " << (int) dest->GetCryptoKeyType() << std::endl;
		if (keys.IsOfflineSignature ())
		{
			std::cout << "Offline signature" << std::endl;
			const auto& offlineSignature = keys.GetOfflineSignature ();
			std::cout << "Expires: " << ConvertTime (bufbe32toh(offlineSignature.data ())) << std::endl;
			std::cout << "Transient Signature Type: " << SigTypeToName(bufbe16toh(offlineSignature.data () + 4)) << std::endl;
		}
	} else {
		if(print_private) {
			std::cout << keys.ToBase64() << std::endl;
		} else if(print_dest) {
			std::cout << dest->ToBase64() << std::endl;
		} else {
			std::cout << ident.ToBase32() << ".b32.i2p" << std::endl;
		}
	}

	if (print_blinded) {
		if (dest->GetSigningKeyType () == i2p::data::SIGNING_KEY_TYPE_REDDSA_SHA512_ED25519 ||
		    dest->GetSigningKeyType () == i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519 ) // 11 or 7
		{
			i2p::data::BlindedPublicKey blindedKey (dest);
			std::cout << "b33 address: " << blindedKey.ToB33 () << ".b32.i2p" << std::endl;
			std::cout << "Today's store hash: " << blindedKey.GetStoreHash ().ToBase64 () << std::endl;
			printB33OfflineKeys (keys, blindedKey);
		}
		else
			std::cout << "Invalid signature type " << SigTypeToName (dest->GetSigningKeyType ()) << std::endl;
	}

	i2p::crypto::TerminateCrypto ();
	return 0;
}
