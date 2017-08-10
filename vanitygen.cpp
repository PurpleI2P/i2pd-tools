#include "vanity.hpp"
#define CPU_ONLY




static bool check_prefix(const char * buf){
unsigned short size_str=0;
while(*buf)
{
 if(
 *buf < 48 
 || 
 (*buf > 57 && *buf < 65) 
 ||
 (*buf > 64 && *buf < 94) 
 || *buf > 125
 || size_str > 52
 )return false;
size_str++;
*buf++;
}
return true;
}


#ifdef CPU_ONLY
static inline bool NotThat(const char * a, const char *b){
while(*b)
 if(*a++!=*b++) return true;
return false;
}

inline void twist_cpu(uint8_t * buf){
//TODO: NORMAL IMPLEMENTATION,\
As in miner...

	RAND_bytes(buf,padding_size);
}


// XXX: make this faster
static inline void mutate_keys_cpu(
	uint8_t * buf,
	uint8_t * padding)
{
  twist_cpu(padding);
  thread_mutex.lock();
  keys.RecalculateIdentHash(buf);
  thread_mutex.unlock();
}


static void thread_find(const char * prefix,int id_thread){

#ifndef _WIN32   
  sched_setaffinity(0, sizeof(cpu), cpu);
  cpu++;
#endif
  std::cout << "Thread " << id_thread << " binded" << std::endl;

  while(NotThat(keys.GetPublic()->GetIdentHash().ToBase32().c_str(),prefix) and !finded)
  {
    //size_t l0 = 0; \
      in future.

    mutate_keys_cpu(KeyBuf,PaddingBuf);
    hash++;
  }
}
#endif


int main (int argc, char * argv[])
{
	if ( argc < 3 )
	{
		std::cout << "Usage: " << argv[0] << " filename generatestring <signature type>" << std::endl;
		return 0;
	}
	if(!check_prefix(argv[2])){
		std::cout << "Not correct prefix" << std::endl;
		return 0;
	}
	i2p::crypto::InitCrypto (false);
	type = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519;	
	if ( argc > 3 ) 
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
/*
TODO:
 <orignal> Francezgy переделай пожалуйста треды
 <orignal> без всех это pthread
 * orignal has quit (Quit: Leaving)
*/
//
  KeyBuf = new uint8_t[keys.GetFullLen()];
  PaddingBuf = keys.GetPadding();
//
  unsigned int count_cpu = sysconf(_SC_NPROCESSORS_ONLN);
  std::vector<std::thread> threads(count_cpu);
  INIT_CPUS(count_cpu);
//
  std::cout << "Start vanity generator in " << count_cpu << " threads" << std::endl;
///
  for ( unsigned int j = count_cpu;j--;)
   threads[j] = std::thread(thread_find,argv[2],j);

  // SET AFFINITY NOW IN FUNCTION

  for(unsigned int j = 0; j < count_cpu;j++)
   threads[j].join();
///
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

