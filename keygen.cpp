#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Crypto.h"
#include "Identity.h"
#include "common/key.hpp"

int main (int argc, char * argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: keygen filename <signature type>" << std::endl;
		return -1;
	}
	i2p::crypto::InitCrypto (false, true, false);
	i2p::data::SigningKeyType type = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519;
	if (argc > 2) {
		std::string str(argv[2]);
		type = NameToSigType(str);
		if (SigTypeToName(type).find("unknown") != std::string::npos) { std::cerr << "Incorrect signature type" << std::endl; return -2; }
	}
	auto keys = i2p::data::PrivateKeys::CreateRandomKeys (type);
	std::ofstream f (argv[1], std::ofstream::binary | std::ofstream::out);
	if (f)
	{
		size_t len = keys.GetFullLen ();
		uint8_t * buf = new uint8_t[len];
		len = keys.ToBuffer (buf, len);
		f.write ((char *)buf, len);
		delete[] buf;
		std::cout << "Destination " << keys.GetPublic ()->GetIdentHash ().ToBase32 () << " created" << std::endl;
		std::cout << "Signature type: " << SigTypeToName(type) << " (" << type << ")" << std::endl;
	}
	else
		std::cout << "Can't create file " << argv[1] << std::endl;

	i2p::crypto::TerminateCrypto ();

	return 0;
}


