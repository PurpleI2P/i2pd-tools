#include <openssl/evp.h> 
#include <openssl/bn.h>
#include <iostream>
#include <string>
#include <iomanip>
#include "Base.h"

#define KEYSIZE 32
size_t len = KEYSIZE;

struct BoxKeys
{
    uint8_t PublicKey[KEYSIZE];
    uint8_t PrivateKey[KEYSIZE];
};

BoxKeys getKeyPair()
{
    BoxKeys keys;

    EVP_PKEY_CTX * Ctx;
    EVP_PKEY * Pkey = nullptr;
    Ctx = EVP_PKEY_CTX_new_id (NID_X25519, NULL);

    EVP_PKEY_keygen_init (Ctx);
    EVP_PKEY_keygen (Ctx, &Pkey);

    EVP_PKEY_get_raw_public_key (Pkey, keys.PublicKey, &len);
    EVP_PKEY_get_raw_private_key (Pkey, keys.PrivateKey, &len);

    EVP_PKEY_CTX_free(Ctx);
    EVP_PKEY_free(Pkey);

    return keys;
}

int main(int argc, char * argv[])
{
    if (argc > 1)
    {
        std::string arg = static_cast<std::string>(argv[1]);
        if (arg == "--usage" || arg == "--help" || arg == "-h")
        {
            std::cout << "The x25519 keys are used for authentication with an encrypted LeaseSet.\n"
            << "Server side:\n"
            << "  signaturetype = 11\n"
            << "  i2cp.leaseSetType = 5\n"
            << "  i2cp.leaseSetAuthType = 1\n"
            << "  i2cp.leaseSetClient.dh.210 = clientName:PublicKey\n"
            << "Client side:\n"
            << "  i2cp.leaseSetPrivKey = PrivateKey\n\n"
            << "https://i2pd.readthedocs.io/en/latest/user-guide/tunnels/" << std::endl;

            return 0;
        }
    }

    BoxKeys newKeys = getKeyPair();

    size_t len_out = 50;
    char b64Public[len_out] = {0};
    char b64Private[len_out] = {0};

    i2p::data::ByteStreamToBase64 (newKeys.PublicKey, len, b64Public, len_out);

    std::cout << "PublicKey: ";
    for (int i = 0; b64Public[i] != 0; ++i)
        std::cout << b64Public[i];

    i2p::data::ByteStreamToBase64 (newKeys.PrivateKey, len, b64Private, len_out);

    std::cout << "\nPrivateKey: ";
    for (int i = 0; b64Private[i] != 0; ++i)
        std::cout << b64Private[i];
    std::cout << std::endl;

    return 0;
}
