#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include "Identity.h"
#include "Base.h"

static void help ()
{
	std::cout << "Usage:" << std::endl;
	std::cout << "\treg3ldaddr step1 privkey   address" << std::endl;
	std::cout << "\treg3ldaddr step2 step1file oldprivkey oldaddress" << std::endl;
	std::cout << "\treg3ldaddr step3 step2file privkey" << std::endl;
}

int main (int argc, char * argv[])
{
	if (argc < 3) {	help(); return -1;}
	std::string arg = argv[1];

	i2p::crypto::InitCrypto (false);
	i2p::data::PrivateKeys keys;

	if (arg == "step1") {
		std::ifstream s(argv[2], std::ifstream::binary);
		if (s.is_open ()) {
			s.seekg (0, std::ios::end);
			size_t len = s.tellg();
			s.seekg (0, std::ios::beg);
			uint8_t * buf = new uint8_t[len];
			s.read ((char *)buf, len);
			if(keys.FromBuffer (buf, len)) {
				std::stringstream out;
				out << argv[3] << "="; // address
				out << keys.GetPublic ()->ToBase64 ();
				out << "#!action=addsubdomain";
				std::cout << out.str () << std::endl;
			} else
				std::cout << "Failed to load keyfile " << argv[1] << std::endl;
			delete[] buf;
		}
	}
	else if (arg == "step2") {
		std::ifstream t(argv[2]);
		std::ifstream s(argv[3], std::ifstream::binary);
		std::string regtxt;
		std::stringstream out;

		if (t.is_open ()) {
			while (t.good()) {
				getline (t, regtxt);
				out << regtxt;
			}
			t.close();
		} else {
			std::cout << "Failed to read file with STEP1 output";
			exit(1);
		}

		if (s.is_open ()) {
			s.seekg (0, std::ios::end);
			size_t len = s.tellg();
			s.seekg (0, std::ios::beg);
			uint8_t * buf = new uint8_t[len];
			s.read ((char *)buf, len);
			if(keys.FromBuffer (buf, len)) {
				auto signatureLen = keys.GetPublic ()->GetSignatureLen ();
				uint8_t * signature = new uint8_t[signatureLen];
				//char * sig = new char[signatureLen*2];
				out << "#date=" << std::time(nullptr);
				out << "#olddest=" << keys.GetPublic ()->ToBase64 ();
				out << "#oldname=" << argv[4];
				keys.Sign ((uint8_t *)out.str ().c_str (), out.str ().length (), signature);
				auto sig = i2p::data::ByteStreamToBase64 (signature, signatureLen);//, sig, signatureLen*2);
				//sig[len] = 0;
				out << "#oldsig=" << sig;
				delete[] signature;
				//delete[] sig;
				std::cout << out.str () << std::endl;
			} else
				std::cout << "Failed to load keyfile " << argv[1] << std::endl;
			delete[] buf;
		}
	}
	else if (arg == "step3") {
		std::ifstream t(argv[2]);
		std::ifstream s(argv[3], std::ifstream::binary);
		std::string regtxt;
		std::stringstream out;

		if (t.is_open ()) {
			while (t.good()) {
				getline (t, regtxt);
				out << regtxt;
			}
			t.close();
		} else {
			std::cout << "Failed to read file with STEP2 output";
			exit(1);
		}

		if (s.is_open ()) {
			s.seekg (0, std::ios::end);
			size_t len = s.tellg();
			s.seekg (0, std::ios::beg);
			uint8_t * buf = new uint8_t[len];
			s.read ((char *)buf, len);
			if(keys.FromBuffer (buf, len)) {
				auto signatureLen = keys.GetPublic ()->GetSignatureLen ();
				uint8_t * signature = new uint8_t[signatureLen];
				//char * sig = new char[signatureLen*2];
				keys.Sign ((uint8_t *)out.str ().c_str (), out.str ().length (), signature);
				auto sig = i2p::data::ByteStreamToBase64 (signature, signatureLen);//, sig, signatureLen*2);
				//sig[len] = 0;
				out << "#sig=" << sig;
				delete[] signature;
				//delete[] sig;
				std::cout << out.str () << std::endl;
			} else
				std::cout << "Failed to load keyfile " << argv[1] << std::endl;
			delete[] buf;
		}
	}
	else {
		help(); exit(1);
	}

	i2p::crypto::TerminateCrypto ();
	return 0;
}
