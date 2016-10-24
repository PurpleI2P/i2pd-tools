/**
 * famtool - a tool for creating and verifying router families
 */
#include <cassert>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "Crypto.h"
#include "RouterInfo.h"
#include "Base.h"
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ssl.h>

using namespace i2p::crypto;
using namespace i2p::data;

static void usage(const std::string & name)
{
  std::cout << "usage: " << name << " [-h] [-v] [-g -n family -c family.crt -k family.pem] [-s -n family -k family.pem -i router.keys -f router.info] [-V -c family.crt -f router.info]" << std::endl;
}

static void printhelp(const std::string & name)
{
  usage(name);
  std::cout << std::endl;
  std::cout << "generate a new family signing key for family called ``i2pfam''" << std::endl;
  std::cout << name << " -g -n i2pfam -c myfam.crt -k myfam.pem" << std::endl << std::endl;
  std::cout << "sign a router info with family signing key" << std::endl;
  std::cout << name << " -s -n i2pfam -k myfam.pem -i router.keys -f router.info" << std::endl << std::endl;
  std::cout << "verify signed router.info" << std::endl;
  std::cout << name << " -V -n i2pfam -c myfam.pem -f router.info" << std::endl << std::endl;
}

static std::shared_ptr<Verifier> LoadCertificate (const std::string& filename)
{
  std::shared_ptr<Verifier> verifier;
  SSL_CTX * ctx = SSL_CTX_new (TLSv1_method ());
  int ret = SSL_CTX_use_certificate_file (ctx, filename.c_str (), SSL_FILETYPE_PEM); 
  if (ret)
  {	
    SSL * ssl = SSL_new (ctx);
    X509 * cert = SSL_get_certificate (ssl);
    if (cert)
    {	
      // extract issuer name
      char name[100];
      X509_NAME_oneline (X509_get_issuer_name(cert), name, 100);
      char * cn = strstr (name, "CN=");
  		if (cn)
			{	
        cn += 3;
        char * family = strstr (cn, ".family");
        if (family) family[0] = 0;
      }	
      auto pkey = X509_get_pubkey (cert);
      int keyType = EVP_PKEY_type(pkey->type);
      switch (keyType)
  		{
        case EVP_PKEY_EC:
        {
          EC_KEY * ecKey = EVP_PKEY_get1_EC_KEY (pkey);
          if (ecKey)
          {
            auto group = EC_KEY_get0_group (ecKey);
            if (group)
            {
              int curve = EC_GROUP_get_curve_name (group);
              if (curve == NID_X9_62_prime256v1)
  						{
                uint8_t signingKey[64];
                BIGNUM * x = BN_new(), * y = BN_new();
                EC_POINT_get_affine_coordinates_GFp (group,
										EC_KEY_get0_public_key (ecKey), x, y, NULL);
                bn2buf (x, signingKey, 32);
                bn2buf (y, signingKey + 32, 32);
                BN_free (x); BN_free (y);
                verifier = std::make_shared<ECDSAP256Verifier>(signingKey);
              }
            }
            EC_KEY_free (ecKey);
          }
        }
      
      default:
        break;
      }
      EVP_PKEY_free (pkey);
    }
    SSL_free (ssl);			
  }
  SSL_CTX_free (ctx);
  return verifier;
}

static bool CreateFamilySignature (const std::string& family, const IdentHash& ident, const std::string & filename, std::string & sig)
{
  SSL_CTX * ctx = SSL_CTX_new (TLSv1_method ());
  int ret = SSL_CTX_use_PrivateKey_file (ctx, filename.c_str (), SSL_FILETYPE_PEM); 
  if (ret)
		{
			SSL * ssl = SSL_new (ctx);
			EVP_PKEY * pkey = SSL_get_privatekey (ssl);
			EC_KEY * ecKey = EVP_PKEY_get1_EC_KEY (pkey);
			if (ecKey)
        {
          auto group = EC_KEY_get0_group (ecKey);
          if (group)
            {
              int curve = EC_GROUP_get_curve_name (group);
              if (curve == NID_X9_62_prime256v1)
                {
                  uint8_t signingPrivateKey[32], buf[50], signature[64];
                  bn2buf (EC_KEY_get0_private_key (ecKey), signingPrivateKey, 32);
                  ECDSAP256Signer signer (signingPrivateKey);
                  size_t len = family.length ();
                  memcpy (buf, family.c_str (), len);
                  memcpy (buf + len, (const uint8_t *)ident, 32);
                  len += 32;
                  signer.Sign (buf, len, signature);
                  len = Base64EncodingBufferSize (64);
                  char * b64 = new char[len+1];
                  len = ByteStreamToBase64 (signature, 64, b64, len);
                  b64[len] = 0;
                  sig = b64;
                  delete[] b64;
                }
              else
                return false;
            }	
        }	
			SSL_free (ssl);		
		}	
  else
    return false;
  SSL_CTX_free (ctx);	
  return true;
}	

int main(int argc, char * argv[])
{
  if (argc == 1) {
    usage(argv[0]);
    return -1;
  }
  int opt;
  bool verbose = false;
  bool help = false;
  bool gen = false;
  bool sign = false;
  bool verify = false;
  std::string fam;
  std::string privkey;
  std::string certfile;
  std::string infile;
  std::string infofile;
  std::string outfile;
  while((opt = getopt(argc, argv, "vVhgsn:i:c:k:o:f:")) != -1) {
    switch(opt) {
    case 'v':
      verbose = true;
      break;
    case 'h':
      help = true;
      break;
    case 'g':
      gen = true;
      break;
    case 'n':
      fam = std::string(argv[optind-1]);
      if (fam.size() + 32 > 50) {
        std::cout << "family name too long" << std::endl;
        return 1;
      }
      break;
    case 'f':
      infofile = std::string(argv[optind-1]);
      break;
    case 'i':
      infile = std::string(argv[optind-1]);
      break;
    case 'o':
      outfile = std::string(argv[optind-1]);
    case 'c':
      certfile = std::string(argv[optind-1]);
      break;
    case 'k':
      privkey = std::string(argv[optind-1]);
      break;
    case 'V':
      verify = true;
      break;
    case 's':
      sign = true;
      break;
    default:
      usage(argv[0]);
      return -1;
    }
  }
  if(help) {
    printhelp(argv[0]);
    return 0;
  }
  
  InitCrypto(false);
  
  if(!fam.size()) {
    // no family name
    std::cout << "no family name specified" << std::endl;
    return 1;
  }
  // generate family key code
  if(gen) {
    if(!privkey.size()) privkey = fam + ".key";
    if(!certfile.size()) certfile = fam + ".crt";
    
    std::string cn = fam + ".family.i2p.net";


    FILE * privf = fopen(privkey.c_str(), "w");
    if(!privf) {
      fprintf(stderr, "cannot open %s: %s\n", privkey.c_str(), strerror(errno));
      return 1;
    }

    FILE * certf = fopen(certfile.c_str(), "w");
    if(!certf) {
      fprintf(stderr, "cannot open %s: %s\n", certfile.c_str(), strerror(errno));
      return 1;
    }

    // openssl fagmastery starts here
    
    EC_KEY * k_priv = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    assert(k_priv);
    EC_KEY_set_asn1_flag(k_priv, OPENSSL_EC_NAMED_CURVE);
    EC_KEY_generate_key(k_priv);
    if(verbose) std::cout << "generated key" << std::endl;
    // TODO: password protection
    PEM_write_ECPrivateKey(privf, k_priv, nullptr, nullptr, 0, nullptr, nullptr);
    fclose(privf);
    if(verbose) std::cout << "wrote private key" << std::endl;
    
    
    EVP_PKEY * ev_k = EVP_PKEY_new();
    assert(ev_k);
    assert(EVP_PKEY_assign_EC_KEY(ev_k, k_priv) == 1);
    
    X509 * x = X509_new();
    assert(x);
    
    X509_set_version(x, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x), 0);
    X509_gmtime_adj(X509_get_notBefore(x),0);
    // TODO: make expiration date configurable
    X509_gmtime_adj(X509_get_notAfter(x),(long)60*60*24*365*10);
    
    X509_set_pubkey(x, ev_k);
    
    X509_NAME * name = X509_get_subject_name(x);
    X509_NAME_add_entry_by_txt(name,"C", MBSTRING_ASC, (unsigned char *) "XX", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name,"ST", MBSTRING_ASC, (unsigned char *) "XX", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name,"L", MBSTRING_ASC, (unsigned char *) "XX", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name,"O", MBSTRING_ASC, (unsigned char *) "I2P Anonymous Network", -1, -1, 0);
    
    X509_NAME_add_entry_by_txt(name,"OU", MBSTRING_ASC, (unsigned char *) "family", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name,"CN", MBSTRING_ASC, (unsigned char *) cn.c_str(), -1, -1, 0);
    X509_set_issuer_name(x,name);

    if(verbose) std::cout << "signing cert" << std::endl;
    assert(X509_sign(x, ev_k, EVP_sha256()));
    if(verbose) std::cout << "writing private key" << std::endl;
    PEM_write_X509(certf,  x);

    fclose(certf);
    
    EVP_PKEY_free(ev_k);
    X509_free(x);
    std::cout << "family " << fam << " made" << std::endl;
  }

  if (sign) {
    // sign
    if (!infile.size()) {
      // no router info specififed
      std::cerr << "no router keys file specified" << std::endl;
      return 1;
    }
    if (!privkey.size()) {
      // no private key specified
      std::cerr << "no private key specififed" << std::endl;
      return 1;
    }
    
    {
      std::ifstream i;
      i.open(infofile);
      if(!i.is_open()) {
        std::cout << "cannot open " << infofile << std::endl;
        return 1;
      }
    }

    if (verbose) std::cout << "load " << infofile << std::endl;

    
    
    PrivateKeys keys;
    {
      std::ifstream fi(infile, std::ifstream::in | std::ifstream::binary);
      if(!fi.is_open()) {
        std::cout << "cannot open " << infile << std::endl;
        return 1;
      }
      fi.seekg (0, std::ios::end);
      size_t len = fi.tellg();
      fi.seekg (0, std::ios::beg);		
      uint8_t * k = new uint8_t[len];
      fi.read((char*)k, len);
      if(!keys.FromBuffer(k, len)) {
        std::cout << "invalid key file " << infile << std::endl;
        return 1;
      }
      delete [] k;
    }
    
    RouterInfo ri(infofile);
    auto ident = ri.GetIdentHash();

    
    if (verbose) std::cout << "add " << ident.ToBase64() << " to " << fam << std::endl;
    std::string sig;
    if(CreateFamilySignature(fam, ident, privkey, sig)) {
      ri.SetProperty(ROUTER_INFO_PROPERTY_FAMILY, fam);
      ri.SetProperty(ROUTER_INFO_PROPERTY_FAMILY_SIG, sig);
      if (verbose) std::cout << "signed " << sig << std::endl;
      ri.CreateBuffer(keys);    
      if(!ri.SaveToFile(infofile)) {
        std::cout << "failed to save to " << infofile << std::endl;
      }
    } else {
      std::cout << "failed to sign router info" << std::endl;
    }
    std::cout << "signed" << std::endl;
  }

  if(verify) {
    if(!infofile.size()) {
      std::cout << "no router info file specified" << std::endl;
      return 1;
    }
    if(!certfile.size()) {
      std::cout << "no family cerifiticate specified" << std::endl;
      return 1;
    }
    auto v = LoadCertificate(certfile);
    if(!v) {
      std::cout << "invalid certificate" << std::endl;
      return 1;
    }

    {
      std::ifstream i;
      i.open(infofile);
      if(!i.is_open()) {
        std::cout << "cannot open " << infofile << std::endl;
        return 1;
      }
    }

    if (verbose) std::cout << "load " << infofile << std::endl;
    
    RouterInfo ri(infofile);
    auto sig = ri.GetProperty(ROUTER_INFO_PROPERTY_FAMILY_SIG);
    if (ri.GetProperty(ROUTER_INFO_PROPERTY_FAMILY) != fam) {
      std::cout << infofile << " does not belong to " << fam << std::endl;
      return 1;
    }
    auto ident = ri.GetIdentHash();
    
    uint8_t buf[50];
    size_t len = fam.length();
    memcpy(buf, fam.c_str(), len);
    memcpy(buf + len, (const uint8_t *) ident, 32);
    len += 32;
    uint8_t sigbuf[64];
    Base64ToByteStream(sig.c_str(), sig.length(), sigbuf, 64);
    if(!v->Verify(buf, len, sigbuf)) {
      std::cout << "invalid signature" << std::endl;
      return 1;
    }
    std::cout << "verified" << std::endl;
  }
  return 0;
}
