#include <iostream>
#include <string>
#include <memory>
#include "Identity.h"
#include "LeaseSet.h"
#include "common/key.hpp"

int main(int argc, char * argv[])
{	
	// base64 input, b33 and store key output, 11->11 only		
	std::string base64;
	std::getline (std::cin, base64);
	auto ident = std::make_shared<i2p::data::IdentityEx>();
	if (ident->FromBase64 (base64))
	{
		if (ident->GetSigningKeyType () == i2p::data::SIGNING_KEY_TYPE_REDDSA_SHA512_ED25519)
		{
			i2p::data::BlindedPublicKey blindedKey (ident);
			std::cout << "b33 address: " << blindedKey.ToB33 () << ".b32.i2p" << std::endl;
			std::cout << "Today's store hash: " << blindedKey.GetStoreHash ().ToBase64 () << std::endl;	
		}
		else
			std::cout << "Invalid signature type " << SigTypeToName (ident->GetSigningKeyType ()) << std::endl;	
	}
	else
		std::cout << "Invalid base64 address" << std::endl;	

	return 0;
}

