#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <cassert>
#include "Base.h"

const size_t BUFFSZ = 1024;

static int printHelp(const char * exe, int exitcode)
{
  std::cout << "usage: " << exe << " [-d] [filename]" << std::endl;
  return exitcode;
}

template <typename InCh, typename OutCh, size_t isz, size_t osz>
static int operate(std::function<std::size_t(const InCh *, size_t, OutCh *, size_t)> f, int infile, int outfile)
{
  InCh inbuf[isz];
  OutCh outbuf[osz];
  ssize_t sz;
  size_t outsz;
  while((sz = read(infile, inbuf, sizeof(inbuf))) > 0)
  {
    outsz = f(inbuf, sz, outbuf, sizeof(outbuf));
    if(outsz && outsz <= sizeof(outbuf))
    {
      write(outfile, outbuf, outsz);
    }
    else
    {
      return -1;
    }
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
    retcode = operate<char, uint8_t, BUFFSZ*4, BUFFSZ*3>(i2p::data::Base64ToByteStream, infile, 1);
  }
  else
  {
    retcode = operate<uint8_t, char, BUFFSZ*3, BUFFSZ*4>(&i2p::data::ByteStreamToBase64, infile, 1);
  }
  close(infile);
  return retcode;
}
