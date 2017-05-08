#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <thread>

#include <iostream>
#include <fstream>
#include "Crypto.h"
#include "Identity.h"
#include "common/key.hpp"

#define error(msg) \
std::cout << "Error: " << msg << std::endl;\
i2p::crypto::TerminateCrypto ();\
return -1;

#define SET_CPUS(cpus,num_proc)\
CPU_ZERO(&cpus);\
for(unsigned int i = num_proc;i--;)\
 CPU_SET(i, &cpus);


static i2p::data::PrivateKeys keys;
static i2p::data::SigningKeyType type;

static cpu_set_t cpus;
static unsigned num_proc;
static volatile bool all=false;
void generate_dest(const char * whatFind)
{
std::cout << "Start a thread" << std::endl;
i2p::data::PrivateKeys tmp_key = i2p::data::PrivateKeys::CreateRandomKeys (type);

while( tmp_key.GetPublic ()->GetIdentHash ().ToBase32 ().find( whatFind ) == -1 )

{
    if(all) break;
    tmp_key = i2p::data::PrivateKeys::CreateRandomKeys (type);
}

keys = tmp_key;

}

int main (int argc, char * argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: keygen filename <signature type> <string>" << std::endl;
		return -1;
	}

	i2p::crypto::InitCrypto (false);
	i2p::data::SigningKeyType type = i2p::data::SIGNING_KEY_TYPE_DSA_SHA1;		
	if (argc >= 3) {
		std::string str(argv[2]);
		type = NameToSigType(str);
	}
	std::cout << "Start generate\n";

	if(argc == 4)
	{

	 num_proc = std::thread::hardware_concurrency();	
	 pthread_t thr[num_proc];

	 SET_CPUS(cpus,num_proc);

	 for( unsigned int i = num_proc;i--;)
	{
	  pthread_create(&thr[i],NULL,generate_dest,argv[3]);
	  pthread_setaffinity_np(thr[i], sizeof(cpu_set_t), &cpus);
	}

	 for(unsigned int i = num_proc;i--;)
	  pthread_join(thr[i],NULL);

	 CPU_ZERO(&cpus);
	} // if(argc==4)
	else
	 keys = i2p::data::PrivateKeys::CreateRandomKeys (type);

	std::cout << "Write to file\n";

	std::ofstream f (argv[1], std::ofstream::binary | std::ofstream::out);
	if (f)
	{
		size_t len = keys.GetFullLen ();
		uint8_t * buf = new uint8_t[len];
		len = keys.ToBuffer (buf, len);
		f.write ((char *)buf, len);
		std::cout << "Destination " << keys.GetPublic ()->GetIdentHash ().ToBase32 () << " created" << std::endl;
		delete[] buf;
	}
	else
		error("Can't create file");	

	i2p::crypto::TerminateCrypto ();
	
	return 0;
}


