# i2pd-tools

This repository contains tools that supplement i2pd.

Notice: git submodules are used so make sure to clone this repository recursively

    git clone --recursive https://github.com/purplei2p/i2pd-tools

## Building

### Dependencies

* boost chrono

* boost date-time

* boost filesystem

* boost program-options

* libssl

* zlib1g

  (run `dependencies.sh`)

### Building

```sh
git submodule init && git submodule update
git submodule update --init
git pull --recurse-submodules
make
```

## Tools included

### i2pbase64

encode/decode b64 string
example usage:
```
user@computer:~/i2pd-tools$ ./i2pbase64 
encode
ZW5jb2RlCg==^C
user@computer:~/i2pd-tools$ ./i2pbase64 -d
ZW5jb2RlCg==
encode
```
U also can use file

### routerinfo

print information about a router info file

#### usage


print ip and port for router info excluding ipv6

    ./routerinfo -p ~/.i2pd/netDb/r6/routerInfo-blah.dat

print iptables firewall rules to allow 1 nodes in netdb through firewall including ipv6 addresses

    ./routerinfo -6 -f ~/.i2pd/netDb/r6/routerInfo-blah.dat

### keygen

Generate an I2P private key

#### Usage

Make a EDDSA-SHA512-ED25519 destination key

    ./keygen privkey.dat

Make an destination key with a certain key type

    ./keygen privkey.dat <number>

or

    ./keygen privkey.dat <key name>


| key name                             | number |
| ------------------------------------ | ------ |
| DSA-SHA1                             | 0      |
| ECDSA-SHA256-P256                    | 1      |
| ECDSA-SHA384-P384                    | 2      |
| ECDSA-SHA512-P521                    | 3      |
| RSA-SHA256-2048                      | 4      |
| RSA-SHA384-3072                      | 5      |
| RSA-SHA512-4096                      | 6      |
| EDDSA-SHA512-ED25519                 | 7      |
| GOSTR3410_CRYPTO_PRO_A-GOSTR3411-256 | 9      |
| GOSTR3410_TC26_A_512-GOSTR3411-512   | 10     |
| RED25519-SHA512                      | 11     |

For more information on the types of signatures, see the [documentation](https://i2pd.readthedocs.io/en/latest/user-guide/tunnels/#signature-types).
### vain

Vanity generation address.

#### Usage

./vain --usage

#### Time to Generate on a 2.70GHz Processor
| characters| time to generate (approx.) |
| -------------------- | --------------- |
|         1 	       | ~0.082s	     |
|         2	           | ~0.075s	     |
|         3	           | ~0.100s	     |
|         4	           | ~0.394s	     |
|         5	           | ~6.343s	     |
|         6	           | ~1m-5m	 	     |
|         7	           | ~30m	 	     |

### keyinfo

Prints information about an I2P private key

#### Usage

Print just the b32 address for this key

     ./keyinfo privatekey.dat

... just the base64 address

    ./keyinfo -d privatekey.dat

Print all info about the public key

    ./keyinfo -v privatekey.dat

### regaddr

Generate authentication string to register a domain on reg.i2p and stats.i2p

    ./regaddr domain.dat domain.i2p > auth_string.txt
    cat auth_string.txt

Send output of auth_string to http://reg.i2p/add and http://stats.i2p/i2p/addkey.html

To register a subdomain, use [`regaddr_3ld`](#regaddr_3ld)

### regaddr_3ld

Generate authentication string to register a subdomain on reg.i2p and stats.i2p in 3 steps

    ./regaddr_3ld step1 sub_domain.dat sub.domain.i2p > step1.txt
    ./regaddr_3ld step2 step1.txt domain.dat domain.i2p > step2.txt
    ./regaddr_3ld step3 step2.txt sub_domain.dat > step3.txt
    cat step3.txt

Send output of auth_string to http://reg.i2p/add and http://stats.i2p/i2p/addkey.html

### regaddralias

Generate authentication string to register an alias address for existing domain on reg.i2p and stats.i2p

    ./regaddralias domain-oldkeys.dat domain-newkeys.i2p domain > auth_string.txt
    cat auth_string.txt

Send output of auth_string to http://reg.i2p/add and http://stats.i2p/i2p/addkey.html

### x25519

Generate key pair with output in base64 encoding. Now the x25519 keys are used for authentication with an encrypted LeaseSet.

### famtool
[this is program for works with family of routers in i2p-network. ](https://i2pd.readthedocs.io/en/latest/user-guide/family/)
    
usage: ```./famtool [-h] [-v] [-g -n family -c family.crt -k family.pem] [-s -n family -k family.pem -i router.keys -f router.info] [-V -c family.crt -f router.info]```
    
generate a new family signing key for family called ``i2pfam''
```./famtool -g -n i2pfam -c myfam.crt -k myfam.pem```
    
sign a router info with family signing key
```./famtool -s -n i2pfam -k myfam.pem -i router.keys -f router.info```
    
verify signed router.info
```./famtool -V -n i2pfam -c myfam.pem -f router.info```
#### Example of usage
    $ ./famtool -g -n i2pfam -c myfam.crt -k myfam.pem
    family i2pfam made
    $ cat myfam.crt
    -----BEGIN CERTIFICATE-----
    MIIB3TCCAYOgAwIBAgIBADAKBggqhkjOPQQDAjB4MQswCQYDVQQGEwJYWDELMAkG
    A1UECAwCWFgxCzAJBgNVBAcMAlhYMR4wHAYDVQQKDBVJMlAgQW5vbnltb3VzIE5l
    dHdvcmsxDzANBgNVBAsMBmZhbWlseTEeMBwGA1UEAwwVaTJwZmFtLmZhbWlseS5p
    MnAubmV0MB4XDTIzMDczMTE5MjQ1MFoXDTMzMDcyODE5MjQ1MFoweDELMAkGA1UE
    BhMCWFgxCzAJBgNVBAgMAlhYMQswCQYDVQQHDAJYWDEeMBwGA1UECgwVSTJQIEFu
    b255bW91cyBOZXR3b3JrMQ8wDQYDVQQLDAZmYW1pbHkxHjAcBgNVBAMMFWkycGZh
    bS5mYW1pbHkuaTJwLm5ldDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABLejNp1Y
    1tnMGFaUIuzNpNL8B9KvaeSSh+OWhRcHn2x1D8TPCVA4IMS0jiMIodCcdyTmz0Qg
    SXsWDqT2CbBMENQwCgYIKoZIzj0EAwIDSAAwRQIgU58CeHXlluPZNxpmxb7HSHsJ
    STCA8C946oas+8uLU+MCIQDe6Km/h8w+oCh+j6UHobN0EAVtQPQGwkq2XXs1jqX2
    bQ==
    -----END CERTIFICATE-----
    $ cat myfam.pem
    -----BEGIN EC PRIVATE KEY-----
    MHcCAQEEIAFA82vZzZN8nZIOVnadSS73G2NNc5pUsh4qmpK2M0nsoAoGCCqGSM49
    AwEHoUQDQgAEt6M2nVjW2cwYVpQi7M2k0vwH0q9p5JKH45aFFwefbHUPxM8JUDgg
    xLSOIwih0Jx3JObPRCBJexYOpPYJsEwQ1A==
    -----END EC PRIVATE KEY-----

#### AutoConf
A program for help create the config file for i2pd

For a now a better way to manual write the config file

For usage just run ./AutoConf or AutoConf.exe
