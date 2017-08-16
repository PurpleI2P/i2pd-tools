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

// sha256
#define Ch(x, y, z)	((x & (y ^ z)) ^ z)
#define Maj(x, y, z)	((x & (y | z)) | (y & z))
#define SHR(x, n)	(x >> n)
#define ROTR(x, n)	((x >> n) | (x << (32 - n)))
#define S0(x)		(ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)		(ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)		(ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define s1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

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


static std::mutex thread_mutex;
static i2p::data::SigningKeyType type;
//static i2p::data::PrivateKeys keys;
static bool found=false;

static size_t MutateByte;

static uint32_t FoundNonce=0;

static uint8_t * KeyBuf;

//static uint8_t * PaddingBuf;
static unsigned long long hashescounter;
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

