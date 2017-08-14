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



#define MODES

static std::mutex thread_mutex;
static i2p::data::SigningKeyType type;
//static i2p::data::PrivateKeys keys;
static bool found=false;

static size_t MutateByte;

static uint32_t FoundNonce=0;

static uint8_t * KeyBuf;

//static uint8_t * PaddingBuf;
static unsigned long long hashescounter;
#ifdef MODES
unsigned short count_cpu __attribute__((__mode__(QI))); // 1 byte i think don't usefful
#else
unsigned short count_cpu;
#endif
