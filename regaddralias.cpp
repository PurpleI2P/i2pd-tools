#include <iostream>
#include <fstream>
#include <sstream>
#include "Identity.h"
#include "Base.h"

int main (int argc, char * argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: regaddralias oldfilename newfilename address" << std::endl;
		return -1;
	}

	i2p::crypto::InitCrypto (false);

	i2p::data::PrivateKeys oldkeys, newkeys;
	{
		std::ifstream s(argv[1], std::ifstream::binary);
		if (s.is_open ())
		{
			s.seekg (0, std::ios::end);
			size_t len = s.tellg();
			s.seekg (0, std::ios::beg);
			uint8_t * buf = new uint8_t[len];
			s.read ((char *)buf, len);
			if(!oldkeys.FromBuffer (buf, len))
			{
				std::cout << "Failed to load keyfile " << argv[1] << std::endl;
				return -1;
			}
			delete[] buf;
		}
		else
		{
			std::cout << "Can't open keyfile " << argv[1] << std::endl;
			return -1;
		}
	}

	{
		std::ifstream s(argv[2], std::ifstream::binary);
		if (s.is_open ())
		{
			s.seekg (0, std::ios::end);
			size_t len = s.tellg();
			s.seekg (0, std::ios::beg);
			uint8_t * buf = new uint8_t[len];
			s.read ((char *)buf, len);
			if(!newkeys.FromBuffer (buf, len))
			{
				std::cout << "Failed to load keyfile " << argv[2] << std::endl;
				return -1;
			}
			delete[] buf;
		}
		else
		{
			std::cout << "Can't open keyfile " << argv[2] << std::endl;
			return -1;
		}
	}

	std::stringstream out;
	out << argv[3] << "="; // address
	out << newkeys.GetPublic ()->ToBase64 ();
	out << "#!action=adddest#olddest=";
	out << oldkeys.GetPublic ()->ToBase64 ();

	auto oldSignatureLen = oldkeys.GetPublic ()->GetSignatureLen ();
	uint8_t * oldSignature = new uint8_t[oldSignatureLen];
	char * oldSig = new char[oldSignatureLen*2];
	oldkeys.Sign ((uint8_t *)out.str ().c_str (), out.str ().length (), oldSignature);
	auto len = i2p::data::ByteStreamToBase64 (oldSignature, oldSignatureLen, oldSig, oldSignatureLen*2);
	oldSig[len] = 0;
	out << "#oldsig=" << oldSig;
	delete[] oldSignature;
	delete[] oldSig;

	auto signatureLen = newkeys.GetPublic ()->GetSignatureLen ();
	uint8_t * signature = new uint8_t[signatureLen];
	char * sig = new char[signatureLen*2];
	newkeys.Sign ((uint8_t *)out.str ().c_str (), out.str ().length (), signature);
	len = i2p::data::ByteStreamToBase64 (signature, signatureLen, sig, signatureLen*2);
	sig[len] = 0;
	out << "#sig=" << sig;
	delete[] signature;
	delete[] sig;

	std::cout << out.str () << std::endl;

	i2p::crypto::TerminateCrypto ();

	return 0;
}
