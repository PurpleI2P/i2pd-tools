#include <iostream>
#include <fstream>
#include <sstream>
#include "Identity.h"
#include "Base.h"

int main (int argc, char * argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: verifyhost '<host record>'" << std::endl;
		return -1;
	}

	i2p::crypto::InitCrypto (false);

	i2p::data::IdentityEx Identity, OldIdentity;

	std::string str (argv[1]);
	std::size_t pos;

	// get record without command block after "#!"
	pos = str.find ("#!");
	std::string hostStr = str.substr (0, pos);

	// get host base64
	pos = hostStr.find ("=");
	std::string hostBase64 = hostStr.substr (pos + 1);

	// load identity
	if (Identity.FromBase64 (hostBase64))
	{
		// get record without sig key and signature
		pos = str.find ("#sig=");
		if (pos == std::string::npos)
		{
			pos = str.find ("!sig=");
                        if (pos == std::string::npos)
			{
				std::cout << "Destination signature not found." << std::endl;
				return 1;
			}
		}

                int offset = (str[pos - 1] == '#' /* only sig in record */) ? 1 : 0;

		std::string hostNoSig = str.substr (0, pos - offset);
		std::string sig = str.substr (pos + 5); // after "#sig=" till end

		auto signatureLen = Identity.GetSignatureLen ();
		uint8_t * signature = new uint8_t[signatureLen];

		// validate signature
		// size_t Base64ToByteStream (std::string_view base64Str, uint8_t * OutBuffer, size_t len);
		i2p::data::Base64ToByteStream(sig, signature, signatureLen);
		if (!Identity.Verify ((uint8_t *)hostNoSig.c_str (), hostNoSig.length (), signature))
		{
			std::cout << "Invalid destination signature." << std::endl;
			return 1;
		}

		if (str.find ("olddest=") != std::string::npos) // if olddest present
		{
			// get olddest
			pos = str.find ("#olddest=");
			std::string oldDestCut = str.substr (pos + 9);
			pos = oldDestCut.find ("#");
			std::string oldDestBase64 = oldDestCut.substr (0, pos);

			// load identity
			if(!OldIdentity.FromBase64 (oldDestBase64))
			{
				std::cout << "Invalid old destination base64." << std::endl;
				return 1;
			}

			signatureLen = OldIdentity.GetSignatureLen ();
			signature = new uint8_t[signatureLen];

			// get record till oldsig key and oldsig
			pos = str.find ("#oldsig=");
			std::string hostNoOldSig = str.substr (0, pos);

			std::string oldSigCut = str.substr (pos + 8);
			pos = oldSigCut.find ("#");
			std::string oldSig = oldSigCut.substr (0, pos);

			// validate signature
			i2p::data::Base64ToByteStream(oldSig, signature, signatureLen);
			bool oldSignValid = OldIdentity.Verify ((uint8_t *)hostNoOldSig.c_str (), hostNoOldSig.length (), signature);

			if(!oldSignValid)
			{
				std::cout << "Invalid old destination signature." << std::endl;
				return 1;
			} else {
				std::cout << "Valid destination" << std::endl;
			}
		}
	}
	else
	{
		std::cout << "Invalid destination base64." << std::endl;
		return 1;
	}

	i2p::crypto::TerminateCrypto ();

	return 0;
}
