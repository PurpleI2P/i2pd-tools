#include <iostream>
#include <unistd.h>
#include "Crypto.h"
#include "RouterInfo.h"


static void usage(const char * argv)
{
	std::cout << "usage: " << argv << " [-6|-f|-p|-y] routerinfo.dat" << std::endl;
}

template<typename Addr>
static std::string address_style_string(Addr addr)
{
	if(addr->transportStyle == i2p::data::RouterInfo::eTransportNTCP2) {
		return "NTCP2";
	} else if (addr->transportStyle == i2p::data::RouterInfo::eTransportSSU2) {
		return "SSU2";
	}
	return "???";

}

template<typename Addr>
static void write_firewall_entry(std::ostream & o, Addr addr)
{

	std::string proto;
	if(addr->transportStyle == i2p::data::RouterInfo::eTransportNTCP2) {
		proto = "tcp";
	} else if (addr->transportStyle == i2p::data::RouterInfo::eTransportSSU2) {
		proto = "udp";
	} else {
		// bail
		return;
	}

	o << " -A OUTPUT -p " << proto;
	o << " -d " << addr->host << " --dport " << addr->port;
	o << " -j ACCEPT";
}

int main(int argc, char * argv[])
{
	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}
	i2p::crypto::InitCrypto(false);
	int opt;
	bool ipv6 = false;
	bool firewall = false;
	bool port = false;
	bool yggdrasil = false;
	while((opt = getopt(argc, argv, "6fpy")) != -1) {
		switch(opt) {
		case '6':
			ipv6 = true;
			break;
		case 'f':
			firewall = true;
			break;
		case 'p':
			port = true;
			break;
		case 'y':
			yggdrasil = true;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	while(optind < argc) {
		int idx = optind;
		optind ++;
		std::string fname(argv[idx]);
		i2p::data::RouterInfo ri(fname);

		std::vector<std::shared_ptr<const i2p::data::RouterInfo::Address> > addrs;
		auto a = ri.GetPublishedNTCP2V4Address();
		if(a)
			addrs.push_back(a);
		a = ri.GetSSU2V4Address();
		if(a)
			addrs.push_back(a);
		if (ipv6)
		{
			a = ri.GetPublishedNTCP2V6Address();
			if(a)
				addrs.push_back(a);
			a = ri.GetSSU2V6Address();
			if(a)
				addrs.push_back(a);
		}

		if(yggdrasil){
			a = ri.GetYggdrasilAddress();
			if(a)
				addrs.push_back(a);
		}

		if(firewall)
			std::cout << "# ";
		else
			std::cout << "Router Hash: ";
		std::cout << ri.GetIdentHashBase64() << std::endl;

		for (const auto & a : addrs) {

			if(firewall) {
				write_firewall_entry(std::cout, a);
			} else {
				std::cout << address_style_string(a) << ": " << a->host;

				if (port)
					std::cout << ":" << a->port;
			}
			std::cout << std::endl;
		}
	}

	return 0;
}
