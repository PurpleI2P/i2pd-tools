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
#ifdef _WIN32
#include <windows.h>
#endif

// MARCO
#ifndef _WIN32   
#define INIT_CPUS(count_cpu){\
cpu = CPU_ALLOC(count_cpu-1);\
if (cpu == NULL) {\
std::cout << "CPU_ALLOC error" << std::endl;\
return 1;\
}\
}
#else
#warning don't testing
#define INIT_CPUS(count_cpu){\
    HANDLE process;\
    DWORD_PTR processAffinityMask;\
    for(int i=0; i<count_cpu; i++) processAffinityMask |= 1<<(2*i);\
    process = GetCurrentProcess();\
    HANDLE thread = GetCurrentThread();\
    DWORD_PTR threadAffinityMask = 1<<(2*omp_get_thread_num());\
    SetThreadAffinityMask(thread, threadAffinityMask);\
}
#endif

static std::mutex thread_mutex;
static i2p::data::SigningKeyType type;
static i2p::data::PrivateKeys keys;
static bool finded=false;
static size_t padding_size;
static uint8_t * KeyBuf;
static uint8_t * PaddingBuf;
static unsigned long long hash;
#ifndef _WIN32   
static cpu_set_t * cpu;
#endif
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

