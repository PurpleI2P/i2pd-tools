#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <cassert>
#include <algorithm>
#include "Base.h"

const size_t BUFFSZ = 1024;

static int printHelp(const char * exe, int exitcode)
{
	std::cout << "usage: " << exe << " [-d] [filename]" << std::endl;
	return exitcode;
}

int operate_b64_decode(int infile, int outfile) {
    constexpr size_t BUFFSZ = 4096;
    char inbuf[BUFFSZ*4];         
    uint8_t outbuf[BUFFSZ*3];     
    ssize_t sz;
    while ((sz = read(infile, inbuf, sizeof(inbuf))) > 0) {
        std::string_view chunk(inbuf, sz);
		std::string s(chunk);
		s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());

        size_t outsz = i2p::data::Base64ToByteStream(s, outbuf, sizeof(outbuf));
        if (outsz > 0) {
            write(outfile, outbuf, outsz);
        } else {
            return -1;
        }
    }
    return errno;
}


int operate_b64_encode(int infile, int outfile) {
    constexpr size_t BUFFSZ = 4096;
    uint8_t inbuf[BUFFSZ*3];     
    //char outbuf[BUFFSZ*4];     
    ssize_t sz;
    while((sz = read(infile, inbuf, sizeof(inbuf))) > 0) {
        std::string out = i2p::data::ByteStreamToBase64(inbuf, sz);
        write(outfile, out.data(), out.size());
    }
    return errno;
}

int main(int argc, char * argv[])
{
	int opt;
	bool decode = false;
	int infile = 0;
	while((opt = getopt(argc, argv, "dh")) != -1)
	{
		switch(opt)
		{
		case 'h':
			return printHelp(argv[0], 0);
		case 'd':
			decode = true;
			break;
		default:
			continue;
		}
	}

	if (argc - optind > 1)
	{
		return printHelp(argv[0], -1);
	}

	if (optind < argc)
	{
		infile = open(argv[optind], O_RDONLY);
		if(infile == -1) {
			perror(argv[optind]);
			return -1;
		}
	}
	int retcode = 0;
	if(decode)
	{
		retcode = operate_b64_decode(infile, 1);
	}
	else
	{
		retcode = operate_b64_encode(infile, 1);
	}
	close(infile);
	return retcode;
}
