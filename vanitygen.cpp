#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <openssl/rand.h>
#include "Crypto.h"
#include "Identity.h"
#include "common/key.hpp"

// XXX: make this faster
static bool check_keys(const std::string & prefix, i2p::data::PrivateKeys & key)
{
  return key.GetPublic()->GetIdentHash().ToBase32().substr(0, prefix.length()) == prefix;
}

// XXX: make this faster
static void mutate_keys(uint8_t * buf, i2p::data::PrivateKeys & key)
{
  uint8_t * ptr = key.GetPadding();
  // TODO: do not hard code for ed25519
  RAND_bytes(ptr, 96);
  key.RecalculateIdentHash(buf);
}

int main (int argc, char * argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " filename prefix" << std::endl;
		return -1;
	}
  uint8_t buf[1024] = {0};
  std::string prefix(argv[2]);
	i2p::crypto::InitCrypto (false);
  // default to ed 25519 keys
	i2p::data::SigningKeyType type = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519;
	auto keys = i2p::data::PrivateKeys::CreateRandomKeys (type);

  // TODO: multi threading
  while(!check_keys(prefix, keys))
    mutate_keys(buf, keys);

	std::ofstream f (argv[1], std::ofstream::binary | std::ofstream::out);
	if (f)
	{
		size_t len = keys.GetFullLen ();
		uint8_t * buf = new uint8_t[len];
		len = keys.ToBuffer (buf, len);
		f.write ((char *)buf, len);
		delete[] buf;
		std::cout << "Destination " << keys.GetPublic ()->GetIdentHash ().ToBase32 () << " created" << std::endl;
	}
	else
		std::cout << "Can't create file " << argv[1] << std::endl;

	i2p::crypto::TerminateCrypto ();

	return 0;
}
