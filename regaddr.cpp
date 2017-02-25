#include <iostream>
#include <fstream>
#include <sstream>
#include "Identity.h"
#include "Base.h"

int main (int argc, char * argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: regaddr filename address" << std::endl;
		return -1;
	}

	i2p::crypto::InitCrypto (false);

	i2p::data::PrivateKeys keys;
	std::ifstream s(argv[1], std::ifstream::binary);
	if (s.is_open ())	
	{	
		s.seekg (0, std::ios::end);
		size_t len = s.tellg();
		s.seekg (0, std::ios::beg);
		uint8_t * buf = new uint8_t[len];
		s.read ((char *)buf, len);
		if(keys.FromBuffer (buf, len))
		{
			auto signatureLen = keys.GetPublic ()->GetSignatureLen ();
			uint8_t * signature = new uint8_t[signatureLen];
			char * sig = new char[signatureLen*2];	
			std::stringstream out;
			out << argv[2] << "="; // address
			out << keys.GetPublic ()->ToBase64 ();
			keys.Sign ((uint8_t *)out.str ().c_str (), out.str ().length (), signature);
			auto len = i2p::data::ByteStreamToBase64 (signature, signatureLen, sig, signatureLen*2);
			sig[len] = 0;
			out << "#!sig=" << sig;
			delete[] signature;
			delete[] sig;
			std::cout << out.str () << std::endl;
		}
		else	
			std::cout << "Failed to load keyfile " << argv[1] << std::endl;
		delete[] buf;
	}		

	i2p::crypto::TerminateCrypto ();

	return 0;
}
