#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <openssl/rand.h>
#include "Crypto.h"
#include "Identity.h"
#include "common/key.hpp"
#include <thread>
#include <unistd.h>
#include <vector>
#include <mutex>
static std::mutex thread_mutex;
static i2p::data::SigningKeyType type;
static i2p::data::PrivateKeys keys;
static bool finded=false;
static size_t padding_size;
static uint8_t * KeyBuf;
static uint8_t * PaddingBuf;
static unsigned long long hash;

#define CPU_ONLY


#ifdef CPU_ONLY
// XXX: make this faster
static inline bool NotThat(const char * a, const char *b){
while(*b)
 if(*a++!=*b++) return true;
return false;
}

inline void twist_cpu(uint8_t * buf,size_t * l0){
//TODO: NORMAL IMPLEMENTATION
	RAND_bytes(buf,padding_size);
}


// XXX: make this faster
static inline void mutate_keys_cpu(
	uint8_t * buf,
	uint8_t * padding,
	size_t * l0)
{
  twist_cpu(padding,l0);
  thread_mutex.lock();
  keys.RecalculateIdentHash(buf);
  thread_mutex.unlock();
}


void thread_find(const char * prefix){
  while(NotThat(keys.GetPublic()->GetIdentHash().ToBase32().c_str(),prefix) and !finded)
  {
    size_t l0 = 0;

    mutate_keys_cpu(KeyBuf,PaddingBuf, (size_t*)&l0);
    hash++;
  }
}
#endif
int main (int argc, char * argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " filename generatestring <signature type>" << std::endl;
		return 0;
	}
	i2p::crypto::InitCrypto (false);
	type = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519;	
	if (argc > 3) 
		type = NameToSigType(std::string(argv[3]));
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			keys = i2p::data::PrivateKeys::CreateRandomKeys (type);
			switch(type){
			case i2p::data::SIGNING_KEY_TYPE_DSA_SHA1:
			case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA512_P521:	
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA256_2048:
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA384_3072:		
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA512_4096:
			case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_TC26_A_512_GOSTR3411_512:
			case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_TC26_A_512_GOSTR3411_512_TEST:
			std::cout << "Sorry, i don't can generate adress for this signature type" << std::endl;
			return 0;			
			break;							
			}
			switch(type){
			case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA256_P256:
			padding_size=64;
			break;
			case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA384_P384:
			padding_size=32;
			break;
			case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA512_P521:
			padding_size=4;
			break;
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA256_2048:
			padding_size=128;
			break;
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA384_3072:
			padding_size=256;
			break;
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA512_4096:
			padding_size=384;
			break;
			case i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519:
			padding_size=96;
			break;
			case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_CRYPTO_PRO_A_GOSTR3411_256:
			case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_CRYPTO_PRO_A_GOSTR3411_256_TEST:
			padding_size=64;
			break;
			}

  // TODO: multi threading
  KeyBuf = new uint8_t[keys.GetFullLen()];
  PaddingBuf = keys.GetPadding();
  unsigned int count_cpu = sysconf(_SC_NPROCESSORS_ONLN);
  std::vector<std::thread> threads(count_cpu);
  std::cout << "Start vanity generator in " << count_cpu << " threads" << std::endl;
  for ( unsigned int j = count_cpu;j--;){
   threads[j] = std::thread(thread_find,argv[2]);
    sched_param sch;
    int policy; 
    pthread_getschedparam(threads[j].native_handle(), &policy, &sch);
    sch.sched_priority = 10;
    if (pthread_setschedparam(threads[j].native_handle(), SCHED_FIFO, &sch)) {
        std::cout << "Failed to setschedparam" << std::endl;
        return 1;
    }
  }
  for(unsigned int j = 0; j < count_cpu;j++)
   threads[j].join();

  std::cout << "Hashes: " << hash << std::endl;
  
	std::ofstream f (argv[1], std::ofstream::binary | std::ofstream::out);
	if (f)
	{
		size_t len = keys.GetFullLen ();
		len = keys.ToBuffer (KeyBuf, len);
		f.write ((char *)KeyBuf, len);
   		delete [] KeyBuf;
		std::cout << "Destination " << keys.GetPublic ()->GetIdentHash ().ToBase32 () << " created" << std::endl;
	}
	else
		std::cout << "Can't create file " << argv[1] << std::endl;

	i2p::crypto::TerminateCrypto ();

	return 0;
}
