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


static size_t ByteStreamToBase32 (const uint8_t * inBuf, size_t len, char * outBuf, size_t outLen)
{
	size_t ret = 0, pos = 1;
	int bits = 8, tmp = inBuf[0];
	while (ret < outLen && (bits > 0 || pos < len))
	{ 	
		if (bits < 5)
		{
			if (pos < len)
			{
				tmp <<= 8;
	      		tmp |= inBuf[pos] & 0xFF;
				pos++;
	      		bits += 8;
			}
			else // last byte
			{
				tmp <<= (5 - bits);
			  	bits = 5;
			}
		}	
	
		bits -= 5;
		int ind = (tmp >> bits) & 0x1F;
		outBuf[ret] = (ind < 26) ? (ind + 'a') : ((ind - 26) + '2');
		ret++;
	}
	outBuf[ret]='\0';
	return ret;
}

static inline bool NotThat(const char * a, const char *b){
while(*b)
 if(*a++!=*b++) return true;
return false;
}


#ifdef CPU_ONLY
static inline bool thread_find(uint8_t * buf,const char * prefix,int id_thread,unsigned long long throughput){
/*
Thanks to orignal ^-^
For idea and example ^-^
Orignal is sensei of crypto ;)
*/
	std::cout << "Thread " << id_thread << " binded" << std::endl;
#ifdef MODES
	uint8_t b[391] __attribute__((__mode__(SI))); // 4 byte == 32 bits, not usefull. i think.
#else
	uint8_t b[391];
#endif
	memcpy (b, buf, 391);

	int len = strlen (prefix);

	SHA256_CTX ctx, ctx1;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, b, MutateByte);

	uint32_t * nonce = (uint32_t *)(b+MutateByte); // in nonce copy of MutateByte of b;
	(*nonce)+=id_thread*throughput;

	uint8_t hash[32];
	char addr[53];

	while(throughput-- and !found){

	memcpy (&ctx1, &ctx, sizeof (SHA256_CTX));
	SHA256_Update(&ctx1, b + MutateByte, 71);
	SHA256_Final(hash, &ctx1);
	ByteStreamToBase32 (hash, 32, addr, len);

	if(	!NotThat(addr,prefix)	){
		ByteStreamToBase32 (hash, 32, addr, 52);
		std::cout << "Address found " << addr << " in " << id_thread << std::endl;
		found=true;
		FoundNonce=*nonce;
		return true;
	}

	(*nonce)++;
	hashescounter++;
	if (found) break;	
	}//while
}

#endif


int main (int argc, char * argv[])
{
	if ( argc < 3 )
	{
		std::cout << "Usage: " << argv[0] << " filename generatestring <threads(default of system)> <signature type>" << std::endl;
		return 0;
	}
	if(!check_prefix(argv[2])){
		std::cout << "Not correct prefix" << std::endl;
		return 0;
	}
	i2p::crypto::InitCrypto (false);
	type = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519;
	if ( argc > 3 ){
		unsigned int tmp = atoi(argv[3]);
		if(tmp > 255) {
			std::cout << "Really more than 255 threads?:D Nope, sorry" << std::endl;
			return 0;
		}
		count_cpu=atoi(argv[3]);
	}if ( argc > 4 ) {
		type = NameToSigType(std::string(argv[4]));
	}

///////////////
//For while
if(type != i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519){
	std::cout << "For a while only ED25519-SHA512" << std::endl;
	return 0;
}
///////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			auto keys = i2p::data::PrivateKeys::CreateRandomKeys (type);
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
//TODO: for other types.
			switch(type){
			case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA256_P256:

			break;
			case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA384_P384:

			break;
			case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA512_P521:
				
			break;
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA256_2048:

			break;
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA384_3072:

			break;
			case i2p::data::SIGNING_KEY_TYPE_RSA_SHA512_4096:

			break;
			case i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519:
			MutateByte=320;
			break;
			case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_CRYPTO_PRO_A_GOSTR3411_256:
			case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_CRYPTO_PRO_A_GOSTR3411_256_TEST:

			break;
			}


  KeyBuf = new uint8_t[keys.GetFullLen()];
  keys.ToBuffer (KeyBuf, keys.GetFullLen ());

  if(!count_cpu)
   count_cpu = sysconf(_SC_NPROCESSORS_ONLN);

   std::cout << "Start vanity generator in " << (int)count_cpu << " threads" << std::endl;


unsigned short attempts = 0;
while(!found)

{//while
{//stack(for destructors(vector/thread))

  std::vector<std::thread> threads(count_cpu);
  unsigned long long thoughtput = 0x4F4B5A37;

  for ( unsigned int j = count_cpu;j--;){
   threads[j] = std::thread(thread_find,KeyBuf,argv[2],j,thoughtput);
   thoughtput+=1000;
  }//for

  for(unsigned int j = 0; j < count_cpu;j++)
   threads[j].join();
  
  if(FoundNonce == 0){
	RAND_bytes( KeyBuf+MutateByte , 90 );
	std::cout << "Attempts #" << ++attempts << std::endl;
   }

}//stack
}//while

  memcpy (KeyBuf + MutateByte, &FoundNonce, 4);
  std::cout << "Hashes: " << hashescounter << std::endl;
  
	std::ofstream f (argv[1], std::ofstream::binary | std::ofstream::out);
	if (f)
	{
		f.write ((char *)KeyBuf, keys.GetFullLen ());
   		delete [] KeyBuf;
	}
	else
		std::cout << "Can't create file " << argv[1] << std::endl;

	i2p::crypto::TerminateCrypto ();

	return 0;
}

#undef MODES

