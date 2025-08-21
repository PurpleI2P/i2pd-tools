#include "vanity.hpp"
#include<regex>
#include<getopt.h>
#include<string>
#include<filesystem>
//#include<boost/algorithm/string/predicate.hpp>
//#include<format> // is not supports for me

// some global vars in vanitygen.hpp
static std::string foundAddress{};
static unsigned short fKeyId = 0;
static struct{
        bool reg=false;
        int threads=-1;
        i2p::data::SigningKeyType signature;
        std::string outputpath="";
        std::regex regex;

}options;
static unsigned short attempts = 0;// it can be disabled, it's just for a statistic. For CPU this is a trash?

static void inline CalculateW (const uint8_t block[64], uint32_t W[64])
{
/*
implementation of orignal
*/
	for (int i = 0; i < 16; i++)
#ifdef _WIN32
		W[i] = htobe32(((uint32_t *)(block))[i]);
#else // from big endian to little endian ( swap )
		W[i] = be32toh(((uint32_t *)(block))[i]);
#endif

	for (int i = 16; i < 64; i++)
		W[i] = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];
}

static void inline TransformBlock (uint32_t state[8], const uint32_t W[64])
{
/*
implementation of orignal
*/
	uint32_t S[8];
	memcpy(S, state, 32);

	uint32_t t0, t1;
	RNDr(S, W, 0, 0x428a2f98); RNDr(S, W, 1, 0x71374491); RNDr(S, W, 2, 0xb5c0fbcf); RNDr(S, W, 3, 0xe9b5dba5);
	RNDr(S, W, 4, 0x3956c25b); RNDr(S, W, 5, 0x59f111f1); RNDr(S, W, 6, 0x923f82a4); RNDr(S, W, 7, 0xab1c5ed5);
	RNDr(S, W, 8, 0xd807aa98); RNDr(S, W, 9, 0x12835b01); RNDr(S, W, 10, 0x243185be); RNDr(S, W, 11, 0x550c7dc3);
	RNDr(S, W, 12, 0x72be5d74); RNDr(S, W, 13, 0x80deb1fe); RNDr(S, W, 14, 0x9bdc06a7); RNDr(S, W, 15, 0xc19bf174);
	RNDr(S, W, 16, 0xe49b69c1); RNDr(S, W, 17, 0xefbe4786); RNDr(S, W, 18, 0x0fc19dc6); RNDr(S, W, 19, 0x240ca1cc);
	RNDr(S, W, 20, 0x2de92c6f); RNDr(S, W, 21, 0x4a7484aa); RNDr(S, W, 22, 0x5cb0a9dc); RNDr(S, W, 23, 0x76f988da);
	RNDr(S, W, 24, 0x983e5152); RNDr(S, W, 25, 0xa831c66d); RNDr(S, W, 26, 0xb00327c8); RNDr(S, W, 27, 0xbf597fc7);
	RNDr(S, W, 28, 0xc6e00bf3); RNDr(S, W, 29, 0xd5a79147); RNDr(S, W, 30, 0x06ca6351); RNDr(S, W, 31, 0x14292967);
	RNDr(S, W, 32, 0x27b70a85); RNDr(S, W, 33, 0x2e1b2138); RNDr(S, W, 34, 0x4d2c6dfc); RNDr(S, W, 35, 0x53380d13);
	RNDr(S, W, 36, 0x650a7354); RNDr(S, W, 37, 0x766a0abb); RNDr(S, W, 38, 0x81c2c92e); RNDr(S, W, 39, 0x92722c85);
	RNDr(S, W, 40, 0xa2bfe8a1); RNDr(S, W, 41, 0xa81a664b); RNDr(S, W, 42, 0xc24b8b70); RNDr(S, W, 43, 0xc76c51a3);
	RNDr(S, W, 44, 0xd192e819); RNDr(S, W, 45, 0xd6990624); RNDr(S, W, 46, 0xf40e3585); RNDr(S, W, 47, 0x106aa070);
	RNDr(S, W, 48, 0x19a4c116); RNDr(S, W, 49, 0x1e376c08); RNDr(S, W, 50, 0x2748774c); RNDr(S, W, 51, 0x34b0bcb5);
	RNDr(S, W, 52, 0x391c0cb3); RNDr(S, W, 53, 0x4ed8aa4a); RNDr(S, W, 54, 0x5b9cca4f); RNDr(S, W, 55, 0x682e6ff3);
	RNDr(S, W, 56, 0x748f82ee); RNDr(S, W, 57, 0x78a5636f); RNDr(S, W, 58, 0x84c87814); RNDr(S, W, 59, 0x8cc70208);
	RNDr(S, W, 60, 0x90befffa); RNDr(S, W, 61, 0xa4506ceb); RNDr(S, W, 62, 0xbef9a3f7); RNDr(S, W, 63, 0xc67178f2);

	for (int i = 0; i < 8; i++)	state[i] += S[i];
}

void inline HashNextBlock (uint32_t state[8], const uint8_t * block)
{
/*
implementation of orignal
*/
	uint32_t W[64];
	CalculateW (block, W);
	TransformBlock (state, W);
}

static bool check_prefix(const char * buf)
{
	unsigned short size_str=0;
	while(*buf)
	{
		if(*buf < 48 || (*buf > 57 && *buf < 65) || (*buf > 64 && *buf < 94) || *buf > 125 || size_str > 52)
			return false;
		size_str++;
		buf++;
	}
	return true;
}

static inline size_t ByteStreamToBase32 (const uint8_t * inBuf, size_t len, char * outBuf, size_t outLen)
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

static inline bool NotThat(const char * what, const std::regex & reg){
	return std::regex_match(what,reg) == 1 ? false : true;
}

static inline bool NotThat(const char * a, const char *b)
{
	while(*b)
		if(*a++!=*b++)
			return true;
	return false;
}

static inline bool thread_find(uint8_t * buf, const char * prefix, int id_thread, unsigned long long throughput)
{
/*
Thanks to orignal ^-^
For idea and example ^-^
Orignal is sensei of crypto ;)
*/
	std::cout << "Thread " << id_thread << " binded" << std::endl;
/*
	union
	{
		uint8_t b[391];
		uint32_t ll;
	} local;
	union
	{
		uint8_t b[32];
		uint32_t ll[8];
	} hash;
*/
	uint8_t b[391]; // b length is 391.
	uint32_t hash[8];

	memcpy (b, buf, 391); // we copy in b our buf, that we give in function.

	auto len = strlen (prefix);
	// precalculate first 5 blocks (320 bytes)
	uint32_t state[8] = { 0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19 };
	HashNextBlock (state, b);
	HashNextBlock (state, b + 64);
	HashNextBlock (state, b + 128);
	HashNextBlock (state, b + 192);
	HashNextBlock (state, b + 256);

	// pre-calculate last W
	uint32_t lastW[64];
	CalculateW (lastBlock, lastW);

	uint32_t * nonce = (uint32_t *)(b+320); // our nonce is place in memory, where is b after 320 bytes (characters)
	(*nonce) += id_thread*throughput;

	char addr[53];
	uint32_t state1[8];

	while(throughput-- and !found)
	{
		memcpy (state1, state, 32);
		// calculate hash of block with nonce
		HashNextBlock (state1, b + 320);
		// apply last block
		TransformBlock (state1, lastW);
		// get final hash
		for (int j = 8; j--;)
			hash[j] = htobe32(state1[j]);
		ByteStreamToBase32 ((uint8_t*)hash, 32, addr, len);
		//	std::cout << addr << std::endl;

		//bool result = options.reg ? !NotThat(addr, &options.regex) : !NotThat(addr,prefix);

		if(	( options.reg ? !NotThat(addr, options.regex) : !NotThat(addr,prefix) ) )
		{
		 	ByteStreamToBase32 ((uint8_t*)hash, 32, addr, 52);
			std::cout << "Address found " << addr << " in " << id_thread << std::endl;
			found=true;
			FoundNonce=*nonce;
			foundAddress = addr;
		 	// From there place we get a nonce, for some one a byte.
			fKeyId = id_thread;
		 	return true;
		 }


		(*nonce)++;
		hashescounter++;
		if (found) // for another threads (?)
		{
			break;
		}
	}//while
	return true;
}





void usage(void){
	constexpr auto help="vain pattern [options]\n"
	"-h --help help menu\n"
	"-r --reg  regexp instead just text pattern\n"
	"--threads -t (default count of system)\n"
//"--signature -s (signature type)\n"
	"-o --output output file(default " DEF_OUT_FILE ")\n"
	"--multiplymode -m - multiple addresses search"
	"";
	puts(help);
}


void parsing(int argc, char ** args){
	int option_index;
	static struct option long_options[]={
		{"help",no_argument,0,'h'},
		{"reg", no_argument,0,'r'},
		{"threads", required_argument, 0, 't'},
		{"signature", required_argument,0,'s'},
		{"output", required_argument,0,'o'},
		{"multiplymode", no_argument, 0, 'm'},
		{0,0,0,0}
	};

	int c;
	while( (c=getopt_long(argc,args, "hrt:s:o:m", long_options, &option_index))!=-1){
		switch(c){
			case 'm':
				multipleSearchMode=true;
				break;
			case 'h':
				usage();
				exit(0);
				break;
			case 'r':
				options.reg=true;
				break;
			case 't':
				options.threads=atoi(optarg);
				break;
			case 's':
				options.signature = NameToSigType(std::string(optarg));
				break;
			case 'o':
				options.outputpath=optarg;
				break;
			case '?':
				std::cerr << "Undefined argument" << std::endl;
			default:
				std::cerr << args[0] << "--help" << std::endl;
				exit(1);
				break;
		}
	}
}

int main (int argc, char * argv[])
{


	

	if ( argc < 2 )
	{
		usage();
		return 0;
	}
	parsing( argc > 2 ? argc-1 : argc, argc > 2 ? argv+1 : argv); // parsing is was there.
	// if argc size more than 2. nameprogram is 1. and 2 is prefix. if not there is will be flags like regex
	// TODO: ?
	if(!options.reg && !check_prefix( argv[1] ))
	{
		std::cout << "Not correct prefix(just string)" << std::endl;
		return 1;
	}else{
		options.regex=std::regex(argv[1]);
	}
// https://github.com/PurpleI2P/i2pd/blob/ae5239de435e1dcdff342961af9b506f60a494d4/libi2pd/Crypto.h#L310
//// init and terminate
//	void InitCrypto (bool precomputation);
// By default false
	i2p::crypto::InitCrypto (PRECOMPUTATION_CRYPTO);
	options.signature = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519;
///////////////
//For while
	if(options.signature != i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519)
	{
		std::cout << "For a while only ED25519-SHA512" << std::endl;
		return 0;
	}
///////////////
// if threads less than 0, then we get from system count of CPUs cores
	if(options.threads <= 0)
	{
	 options.threads = std::thread::hardware_concurrency(); // thx for acetone. lol
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Isntead proccess flipper?
	if ( !std::regex_match( std::string(argv[1]), std::regex("[a-zA-Z0-9\\.]{1,}")) ) {
				std::cerr << "Please, change the outputfile name" << std::endl;
	}
	// 
     	// if ( options . outputpath . empty () ); options . outputpath . assign ( DEF_OUT_FILE ) ;
	static std::string outPutFileName  = options.outputpath;
	auto doSearch = [argc,argv] () {
	 found = false;
     	 // TODO: create libi2pd_tools
     	 // If file not exists we create a dump file. (a bug was found in issues)
     	 switch(options.signature)
     	 {
     		case i2p::data::SIGNING_KEY_TYPE_DSA_SHA1:
     		case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA512_P521:
     		case i2p::data::SIGNING_KEY_TYPE_RSA_SHA256_2048:
     		case i2p::data::SIGNING_KEY_TYPE_RSA_SHA384_3072:
     		case i2p::data::SIGNING_KEY_TYPE_RSA_SHA512_4096:
     		case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_TC26_A_512_GOSTR3411_512:
     		std::cout << "Sorry, i don't can generate adress for this signature type" << std::endl;
     		return 0;
     		break;
     	 }
     
        //TODO: for other types.
     	switch(options.signature)
     	{
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
     		break;
     	}
     // there we gen key to buffer. That we mem allocate...
        const auto keys_len = i2p::data::PrivateKeys::CreateRandomKeys (options.signature).GetFullLen(); // is will be constant. so calculate every time is a bad way
     	auto KeyBufs = new uint8_t*[options.threads];//[keys_len];
	for(auto i = options.threads-1; i--;) {
		KeyBufs[i] = new uint8_t[keys_len];
		auto keys = i2p::data::PrivateKeys::CreateRandomKeys (options.signature);
     		keys.ToBuffer (KeyBufs[i], keys_len);
	}
	
     /// there was some things for cpu 665% usage, but is not helpful even
     	std::cout << "Start vanity generator in " << options.threads << " threads" << std::endl;
     // there we start to change byte in our private key. we can change another bytes too 
     // but we just change 1 byte in all key. So. TODO: change all bytes not one?
     	while(!found)
     	{//while
     		{//stack(for destructors(vector/thread))
     
     			std::vector<std::thread> threads(options.threads);
     			unsigned long long thoughtput = 0x4F4B5A37; // is a magic number. 
     
     			for ( unsigned int j = options.threads;j--;)
     			{
     				// our buf is our key, but in uint8 type, unsigned integ... another argument
     				// is our prefix that we search in address
     				// and j is magic number, is thread id. 
     				// thoughtput is our magic number that we increment on 1000 everytime
     				// so we just change a one a byte in key and convert private key to address
     				// after we check it.
				auto n = j != 0 ? j-1 : 0  ;
				std::cout << "Use " << n << " key" << std::endl;
				
     				threads[j] = std::thread(thread_find,KeyBufs[ n ],argv[1],j,thoughtput);
     				thoughtput+=1000; 
     			}//for
     
     			//There will be proccessFlipper by accetone
     			// if I correctly understand it's drop a payload things in a prefix/search data
     			// or simmilar. We can just use regex. I would to use regex
     		
     			// So I put it ^^^
     			for(unsigned int j = 0; j < (unsigned int)options.threads;j++)
     				threads[j].join();
     			if(FoundNonce == 0)
     			{
     	 			//keys = i2p::data::PrivateKeys::CreateRandomKeys (options.signature);
     				//RAND_bytes( KeyBuf+MutateByte , 90 ); // FoundNonce is
                                DELKEYBUFS(options.threads);
     				std::cout << "(Generate a new keypair) Attempts #" << ++attempts << std::endl;
				return 1;
     			}
     
     		}//stack
     	}//while
     	// before we write result we would to create private.dat a file. dump file. we can use for it keygen
     	// so.
	// std::cout << fKeyId << std::endl;
	auto KeyBuf = KeyBufs[fKeyId - 1 < 0 ? 0 : fKeyId - 1];
     	memcpy (KeyBuf + MutateByte, &FoundNonce, 4);
     	std::cout << "Hashes: " << hashescounter << std::endl;
     
     	// IDK. what for acetone change this line to if (options.output...empty() ... assign
     	// cplusplus.com/reference/string/string/assign yes we can. but I would don't change this
     	//if(options.outputpath.size() == 0) options.outputpath = DEF_OUT_FILE;
		if ( options . outputpath . empty () )
			options.outputpath.assign(foundAddress+".dat");
		else 
			options.outputpath = options.outputpath + std::to_string(foundKeys) + std::string(".dat");
     	while(std::filesystem::exists(options.outputpath))
		{
			options.outputpath.assign(outPutFileName);
			options.outputpath = options.outputpath + std::to_string(foundKeys) + std::string(".dat");
			foundKeys++;
			//printf("foundKeys = %d\n", foundKeys);
		}; 
	//puts("do while cycle break");
	//if ( ! boost::algorithm::ends_with(options.outputpath, ".dat") ) 
	//	options.outputpath = options.outputpath + ".dat";
     
     	// there we generate a key, like as in keygen.cpp
     	// before a mining we would to create a dump file
     	
     	std::cout << "outpath for a now: " << options.outputpath << std::endl;
     
     	std::ofstream f (options.outputpath, std::ofstream::binary | std::ofstream::out);
     	if (f)
     	{
     		f.write ((char *)KeyBuf, keys_len);
		DELKEYBUFS(options.threads);
     	}
     	else
     		std::cout << "Can't create file " << options.outputpath << std::endl;
     	return 0;
     }; // void doSearch lamda

     do {
		doSearch();
		if(found) 
		{
			//TODO: an another variable for file count and found keys as found keys by one runs
			//foundKeys++;
		}
		options.outputpath.assign(outPutFileName);
		FoundNonce = 0;
     } while(multipleSearchMode || !found);

     i2p::crypto::TerminateCrypto ();
     return 0;
}


//
