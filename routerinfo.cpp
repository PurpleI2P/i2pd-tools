#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cctype>
#include "Crypto.h"
#include "RouterInfo.h"

int main(int argc, char * argv[])
{
    if (argc < 2) return 1;
    i2p::crypto::InitCrypto(false);

    for (int i = 1; i < argc; ++i) {
        std::string fname(argv[i]);
        i2p::data::RouterInfo ri(fname);

        std::cout << "Router Hash: " << ri.GetIdentHashBase64() << std::endl;

        const uint8_t* buf = ri.GetBuffer();
        size_t len = ri.GetBufferLen();

        if (buf && len > 0) {
            std::string raw(reinterpret_cast<const char*>(buf), len);
            
            // --- PARSE FROM THE END ---
            // List of specific tags to find by searching backwards
            // Example for my friends from ilita chat:
            // Use any tags you need
            // Insert them in tags below 
            // "router.version=",
            // "caps=",
            // "netId=",
            // "netdb.knownRouters=",
            // "netdb.knownLeaseSets="
            std::vector<std::string> tags = {
                "router.version=", 
                "caps=" 
            };

            for (const auto& tag : tags) {
                // Search from the end of the file
                size_t tag_pos = raw.rfind(tag); 
                
                if (tag_pos != std::string::npos) {
                    // Value starts after the '='
                    size_t v_start = tag_pos + tag.length();
                    // Value ends at the next ';'
                    size_t v_end = raw.find(';', v_start);
                    
                    if (v_end != std::string::npos) {
                        std::string val = raw.substr(v_start, v_end - v_start);
                        std::string key = tag.substr(0, tag.length() - 1);
                        std::cout << "Property: " << key << " = " << val << std::endl;
                    }
                }
            }
        }

        // --- NETWORK TRANSPORTS ---
        auto ntcp2v4 = ri.GetPublishedNTCP2V4Address();
        if (ntcp2v4) std::cout << "NTCP2: " << ntcp2v4->host.to_string() <<  std::endl;

        auto ssu2v4 = ri.GetSSU2V4Address();
        if (ssu2v4) std::cout << "SSU2: " << ssu2v4->host.to_string() << std::endl;

        auto ntcp2v6 = ri.GetPublishedNTCP2V6Address();
        if (ntcp2v6) std::cout << "NTCP2_V6: " << ntcp2v6->host.to_string() <<  std::endl;

        auto ssu2v6 = ri.GetSSU2V6Address();
        if (ssu2v6) std::cout << "SSU2_V6: [" << ssu2v6->host.to_string() << std::endl;
        
    }

    i2p::crypto::TerminateCrypto();
    return 0;
}
