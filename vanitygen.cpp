#include "Crypto.h"
#include "Identity.h"
#include "I2PEndian.h"
#include "common/key.hpp"

#include <regex>
#include <mutex>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <openssl/rand.h>
#include <thread>
#include <unistd.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

// sha256
#define Ch(x, y, z)		((x & (y ^ z)) ^ z)
#define Maj(x, y, z)	((x & (y | z)) | (y & z))
#define SHR(x, n)		(x >> n)
#define ROTR(x, n)		((x >> n) | (x << (32 - n)))
#define S0(x)			(ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)			(ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)			(ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define s1(x)			(ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

#define RND(a, b, c, d, e, f, g, h, k) \
	t0 = h + S1(e) + Ch(e, f, g) + k; \
	t1 = S0(a) + Maj(a, b, c); \
	d += t0; \
	h = t0 + t1;

#define RNDr(S, W, i, k) \
	RND(S[(64 - i) % 8], S[(65 - i) % 8], \
	S[(66 - i) % 8], S[(67 - i) % 8], \
	S[(68 - i) % 8], S[(69 - i) % 8], \
	S[(70 - i) % 8], S[(71 - i) % 8], \
	W[i] + k)

#define DEF_OUTNAME "private.dat"

static bool found = false;
static size_t MutateByte;
static uint8_t * KeyBuf;

unsigned int count_cpu;

const uint8_t lastBlock[64] =
{
	0x05, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x80, // 7 bytes EdDSA certificate 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x38  // 3128 bits (391 bytes)
};

static struct
{
	bool reg = false;
	int threads = -1;
	i2p::data::SigningKeyType signature;
	std::string outputpath = "";
	std::regex regex;
	bool sig_type = true;
} options;

void check_sig_type()
{
	if (SigTypeToName(options.signature).find("unknown") != std::string::npos)
	{
		std::cerr << "Incorrect signature type" << std::endl;
		options.sig_type = false;
	}
}

void inline CalculateW (const uint8_t block[64], uint32_t W[64])
{
/**
 * implementation of orignal
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

void inline TransformBlock (uint32_t state[8], const uint32_t W[64])
{
/**
 * implementation of orignal
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
/**
 * implementation of orignal
 */
	uint32_t W[64];
	CalculateW (block, W);
	TransformBlock (state, W);
}

bool check_prefix(const char * buf)
{
	unsigned short size_str = 0;
	while(*buf)
	{
		if(!((*buf > 49 && *buf < 56) || (*buf > 96 && *buf < 123)) || size_str > 52)
			return false;
		size_str++;
		buf++;
	}
	return true;
}

inline size_t ByteStreamToBase32 (const uint8_t * inBuf, size_t len, char * outBuf, size_t outLen)
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

inline bool NotThat(const char * what, const std::regex & reg){
	return std::regex_match(what,reg) == 1 ? false : true;
}

inline bool NotThat(const char * a, const char *b)
{
	while(*b)
		if(*a++!=*b++)
			return true;
	return false;
}

void processFlipper(const std::string string)
{
    constexpr char SYMBOLS[] {'-', '\\', '|', '/'};
    uint8_t symbol_counter = 0;
    std::string payload = string;
    if (payload.back() != ' ') payload += ' ';
    
    size_t current_state = payload.size();
    enum { left, right } direction = left;
    
    std::cout << payload << SYMBOLS[symbol_counter++];
    while (!found)
    {
        std::cout << '\b' << SYMBOLS[symbol_counter++];
        std::cout.flush();
        
        if (symbol_counter == sizeof(SYMBOLS))
        {
            if (direction == left)
            {
                std::cout << '\b';
                std::cout.flush();
                symbol_counter = 0;
                if (!--current_state)
                {
                    direction = right;
                }
            }
            else if (direction == right)
            {
                std::cout << '\b' << payload[current_state] << " ";
                std::cout.flush();
                symbol_counter = 0;
                
                if (++current_state == payload.size())
                {
                    direction = left;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

bool thread_find(uint8_t * buf, const char * prefix, int id_thread, unsigned long long throughput)
{
    const unsigned long long original_throughput = throughput;
	uint8_t b[391];
	uint32_t hash[8];

	memcpy (b, buf, 391);

	size_t len = 52;
	
	if (!options.reg)
		len = strlen(prefix);

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

	uint32_t * nonce = (uint32_t *)(b+320);
	(*nonce) += id_thread*throughput;

	char addr[53];
	uint32_t state1[8];

	while(!found)
	{
		if (! throughput--)
		{
			throughput = original_throughput;
		}
        
		memcpy (state1, state, 32);
		// calculate hash of block with nonce
		HashNextBlock (state1, b + 320);
		// apply last block
		TransformBlock (state1, lastW);
		// get final hash
		for (int j = 8; j--;)
			hash[j] = htobe32(state1[j]);
		ByteStreamToBase32 ((uint8_t*)hash, 32, addr, len);

		if( options.reg ? !NotThat(addr, options.regex) : !NotThat(addr, prefix) )
		{
			ByteStreamToBase32 ((uint8_t*)hash, 32, addr, 52);
			std::cout << "\nFound address: " << addr << std::endl;
			found = true;
			return true;
		}

		(*nonce)++;
		if (found)
		{
			break;
		}
	}
	
	return true;
}

void usage(void){
	const constexpr char * help="Usage:\n"
    "  vain [text-pattern|regex-pattern] [options]\n\n"
    "OPTIONS:\n"
	"  -h --help      show this help (same as --usage)\n"
	"  -r --reg       use regexp instead of simple text pattern, ex.: vain '(one|two).*' -r\n"
	"  -t --threads   number of threads to use (default: one per processor)\n"
//	"  -s --signature (signature type)\n" // NOT IMPLEMENTED FUCKING PLAZ!
	"  -o --output    privkey output file name (default: ./" DEF_OUTNAME ")\n"
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
		{"usage", no_argument,0,0},
		{0,0,0,0}
	};

	int c;
	while( (c=getopt_long(argc,args, "hrt:s:o:", long_options, &option_index))!=-1){
		switch(c){
			case 0:
				if ( std::string(long_options[option_index].name) == std::string("usage") ){
					usage();
					exit(1);
				}
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
				check_sig_type();
				break;
			case 'o':
				options.outputpath=optarg;
				break;
			case '?':
				std::cerr << "Undefined argument" << std::endl;
			default:
				std::cerr << args[0] << " --usage / --help" << std::endl;
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
    
	parsing( argc > 2 ? argc-1 : argc, argc > 2 ? argv+1 : argv);
	//
	if(!options.reg && !check_prefix( argv[1] ))
	{
		std::cout << "Invalid pattern." << std::endl;
		usage();
		return 1;
	}else{
		options.regex=std::regex(argv[1]);
	}

	i2p::crypto::InitCrypto (false, true, true, false);
	options.signature = i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519;

	if(options.signature != i2p::data::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519)
	{
		std::cout << "ED25519-SHA512 are currently the only signing keys supported." << std::endl;
		return 0;
	}

	if (!options.sig_type) return -2;
	auto keys = i2p::data::PrivateKeys::CreateRandomKeys (options.signature);
	switch(options.signature)
	{
		case i2p::data::SIGNING_KEY_TYPE_DSA_SHA1:
		case i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA512_P521:
		case i2p::data::SIGNING_KEY_TYPE_RSA_SHA256_2048:
		case i2p::data::SIGNING_KEY_TYPE_RSA_SHA384_3072:
		case i2p::data::SIGNING_KEY_TYPE_RSA_SHA512_4096:
		case i2p::data::SIGNING_KEY_TYPE_GOSTR3410_TC26_A_512_GOSTR3411_512:
		std::cout << "Sorry, selected signature type is not supported for address generation." << std::endl;
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

	KeyBuf = new uint8_t[keys.GetFullLen()];
	keys.ToBuffer (KeyBuf, keys.GetFullLen ());

	if(options.threads <= 0)
	{
		options.threads = std::thread::hardware_concurrency();
	}
	
	std::cout << "Vanity generator started in " << options.threads << " threads" << std::endl;

	std::vector<std::thread> threads(options.threads);
	unsigned long long thoughtput = 0x4F4B5A37;

	for (unsigned int j = options.threads; j--; )
	{
		threads[j] = std::thread(thread_find, KeyBuf, argv[1], j, thoughtput);
		thoughtput += 1000;
	}

	processFlipper(argv[1]);
    
	for (unsigned int j = 0; j < (unsigned int)options.threads;j++)
		threads[j].join();

	if(options.outputpath.empty()) options.outputpath.assign(DEF_OUTNAME);

	std::ofstream f (options.outputpath, std::ofstream::binary);
	if (f)
	{
		f.write ((char *)KeyBuf, keys.GetFullLen ());
		delete [] KeyBuf;
	}
	else
		std::cout << "Can't create output file: " << options.outputpath << std::endl;

	i2p::crypto::TerminateCrypto ();

	return 0;
}
