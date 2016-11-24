# i2pd-tools

This repository contains tools that supplement i2pd.

Notice: git submodules are used so make sure to clone this repository recursively

    git clone --recursive https://github.com/purplei2pd/i2pd-tools

## Building

### Dependencies

* boost chrono
* boost date-time
* boost filesystem
* boost program-options
* libssl

```bash
sudo apt-get install \
    libboost-chrono-dev \
    libboost-date-time-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libssl-dev
```

### Building

    make 

## Tools included

### baddiefinder

i2p netdb blocklist generator tool

#### Usage

see [here](baddiefinder)


### keygen

Generate an i2p private key

#### Usage

Make a DSA-SHA1 destination key

    ./keygen privkey.dat

Make an destination key with a certain key type

    ./keygen privkey.dat <number>

or

    ./keygen privkey.dat <key name>


| key name             | number |
| -------------------- | ------ |
| DSA-SHA1             | 0      |
| ECDSA-SHA256-P256    | 1      |
| ECDSA-SHA384-P384    | 2      |
| ECDSA-SHA512-P521    | 3      |
| RSA-SHA256-2048      | 4      |
| RSA-SHA384-3072      | 5      |
| RSA-SHA512-4096      | 6      |
| EDDSA-SHA512-ED25519 | 7      |



### keyinfo

Prints information about an i2p private key

#### Usage

Print just the b32 address for this key

     ./keyinfo privatekey.dat

... just the base64 address

    ./keyinfo -d privatekey.dat

Print all info about the public key

    ./keyinfo -v privatekey.dat
