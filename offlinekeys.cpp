#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <inttypes.h>
#include "Crypto.h"
#include "Identity.h"
#include "Timestamp.h"
#include "common/key.hpp"

int main (int argc, char * argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: offlinekeys <output file> <keys file> <signature type> <days>" << std::endl;
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

	i2p::data::SigningKeyType type = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519; // EdDSA by default
	if (argc > 3) 
	{
		std::string str(argv[3]);
		type = NameToSigType(str);
	}

	int days = 365; // 1 year by default
	if (argc > 4)
		days = std::stoi (argv[4]);	
	uint32_t expires = i2p::util::GetSecondsSinceEpoch () + days*24*60*60;
	
	auto offlineKeys = keys.CreateOfflineKeys (type, expires);	
	std::ofstream f (argv[1], std::ofstream::binary | std::ofstream::out);
	if (f)
	{
		size_t len = offlineKeys.GetFullLen ();
		uint8_t * buf = new uint8_t[len];
		len = offlineKeys.ToBuffer (buf, len);
		f.write ((char *)buf, len);
		delete[] buf;
		std::cout << "Offline keys for destination " << offlineKeys.GetPublic ()->GetIdentHash ().ToBase32 () << " created" << std::endl;
	}
	else
		std::cout << "Can't create file " << argv[1] << std::endl;	

	i2p::crypto::TerminateCrypto ();

	return 0;
}

