#pragma once
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


// MARCO
#define INIT_CPUS(count_cpu){\
cpu = CPU_ALLOC(count_cpu-1);\
if (cpu == NULL) {\
std::cout << "CPU_ALLOC error" << std::endl;\
return 1;\
}\
}




static std::mutex thread_mutex;
static i2p::data::SigningKeyType type;
static i2p::data::PrivateKeys keys;
static bool finded=false;
static size_t padding_size;
static uint8_t * KeyBuf;
static uint8_t * PaddingBuf;
static unsigned long long hash;
static cpu_set_t * cpu;

//Functions visible and don't need.


/*
it does not need, but maybe in future we are can create.
namespace i2p{
namespace tools{
namespace vain{
}
}
}
*/
