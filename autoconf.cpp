#ifndef AUTOCONFC
#define AUTOCONFC
#include<iostream>
#include<sstream>
#include<map>
#include<string>
#include<fstream>
namespace AutoConf {
namespace PreInitConfigs {
	constexpr const char * yggOnlyConf = "ipv4=false\r\n"
										"ipv6=false\r\n"
										"ssu=false\r\n"
										"ntcp2.enabled=false\r\n"
										"ssu2.enabled=false\r\n"
										"meshnets.yggdrasil=true\r\n";
}
//	Texts
using AsksT = std::map<std::string, std::string>;
const std::map<std::string, AsksT> Texts = // maybe vector better
	{
		{"ru", {
			       {"WelcomeText","Привет. Выбери тип конфига\r\n1 - клирнет\r\n2 - только yggdrasil"},
				   // Without section
				   {"TunConfYN", "Использовать другой путь для туннелей?"},
				   {"TunConf", "Введите путь для туннелей"},
				   {"TunnelsDirYN", "Использовать другой путь для папки туннелей?"},
				   {"TunnelsDir","Введите путь для папки с туннелями"},
				   {"certsdirYN", "Использовать другой путь для папки с сертификатами?"},
				   {"certsdir", "Введите путь для папки с сертификатами"},
				   {"pidfileYN","использовать другой путь для PID файла?"},
				   {"pidfile", "Введите путь для pidfile"},
				   {"logYN","Нужно ли изменить путь лога?"},
				   {"log", "Введите тип лога(file,stdout,syslog)"},
				   {"logfileYN","Нужно ли изменить путь logfile?"},
				   {"logfile", "Введите путь logfile"},
				   {"loglevelYN","Нужно ли изменить стандартный уровень логирования?"},
				   {"loglevel","Введите уровень лога(warn,info,none,critical,error,debug)"},
				   {"logCFLYN", "Использовать полный CFL-форму даты в логах? ПО умолчанию только время"},
				   {"daemonYN", "Использовать режим демона?"},
				   {"FamilyUsing", "Введите название фамилии или enter просто"},
				   //TODO: an another
				   {"UseIPv6", "Использовать ipv6?"},
				   {"UseIPv4", "Использовать ipv4?"},
   				   {"BeFloodfill", "Быть флудфиллом?"},
				   {"NoTransit", "Отключить транзит? (это уменьшит анонимность)"}
		       }},
		{"en", {
		       		{"WelcomeText","Hello. Select type of config\r\n1 - clearnet\r\n2 - only yggdrasil"},
			        {"TunConfYN", "Use a different path for tunnels?"},
					{"TunConf", "Enter path for tunnels"},
					{"TunnelsDirYN", "Use a different path for the tunnels folder?"},
					{"TunnelsDir", "Enter path for the tunnels folder"},
					{"certsdirYN", "Use a different path for the certificates folder?"},
					{"certsdir", "Enter path for the certificates folder"},
					{"pidfileYN", "Use a different path for the PID file?"},
					{"pidfile", "Enter path for pidfile"},
					{"logYN", "Do you need to change the log path?"},
					{"log", "Enter log type (file, stdout, syslog)"},
					{"logfileYN", "Do you need to change the logfile path?"},
					{"logfile", "Enter path for logfile"},
					{"loglevelYN", "Do you need to change the default log level?"},
					{"loglevel", "Enter log level (warn, info, none, critical, error, debug)"},
					{"logCFLYN", "Use full CFL format for date in logs? Default is only time."},
					{"daemonYN", "Use daemon mode?"},
					{"FamilyUsing", "Enter your last name or just hit enter."},
				   //TODO: an another
					{"UseIPv6", "Use ipv6?"},
					{"UseIPv4", "Use ipv4?"},
					{"BeFloodfill", "Be a floodfill?"},
					{"NoTransit", "Disable transit? (this will reduce anonymity)"}
		       }}	
	};

// Functions
bool AskYN(void) noexcept {
	char answ; 
	std::cout << " ? (y/n) ";
	std::cin >> answ;
        switch(answ) {
		case 'y':
		case 'Y':
			return true;
		case 'n':
		case 'N':
			return false;
		default:
			return AskYN(); // stack overflow, would use while(true)
	}
}
std::string GetLanguage(void) noexcept {
	std::string lang;
	std::cout << "Language/Язык:\r\nru - русский\r\nen - английский\r\n";
	std::cin >> lang;
	if (Texts.find(lang) != Texts.end()) {
		return lang;
	} else { 
		std::cerr << "Not correct language, try again" << std::endl;
		return GetLanguage(); // stack overflow
	}
}

bool IsOnlyYggdrasil(const std::string & lang) noexcept {
	unsigned short answ;
	std::cout << AutoConf::Texts.at(lang).at("WelcomeText") << std::endl;
	std::cin >> answ; 	
	switch(answ) {
		case 1:
			return false;
		case 2:
			return true;
		default: 
		return IsOnlyYggdrasil(lang);
	}
}

}

int 
main(void) {
	std::cout << "https://i2pd.readthedocs.io/en/latest/user-guide/configuration/\r\nhttps://github.com/PurpleI2P/i2pd/blob/openssl/contrib/i2pd.conf\r\n";
	std::ostringstream conf; 
	auto lang = AutoConf::GetLanguage();
	auto isOnlyYgg = AutoConf::IsOnlyYggdrasil(lang);
	if (isOnlyYgg) {
		#ifndef _WIN32
			conf << "daemon=true\r\n";
		#endif		
		conf << AutoConf::PreInitConfigs::yggOnlyConf;
	} else {
			// Asks
			using namespace AutoConf;
			[](std::ostringstream &conf, const std::string &lang) {
				std::cout << AutoConf::Texts.at(lang).at("TunConfYN") << std::endl;
				if (AskYN()) {
					std::cout << AutoConf::Texts.at(lang).at("TunConf") << "\r\n";
					std::string tunConf;
					std::cin.ignore();
					std::getline(std::cin, tunConf);
					conf << "tunnels.path=" << tunConf << "\r\n";
				}

				std::cout << AutoConf::Texts.at(lang).at("TunnelsDirYN") << std::endl;
				if (AskYN()) {
					std::cout << AutoConf::Texts.at(lang).at("TunnelsDir") << "\r\n";
					std::string tunnelsDir;
					std::cin.ignore();
					std::getline(std::cin, tunnelsDir);
					conf << "tunnels.dir=" << tunnelsDir << "\r\n";
				}

				// TODO:
				{
					std::cout << AutoConf::Texts.at(lang).at("UseIPv6") << "\r\n";
					bool useIPv6 = AskYN();
					conf << "ipv6=" << (useIPv6 ? "true" : "false") << "\r\n";
				}

				{
					std::cout << AutoConf::Texts.at(lang).at("UseIPv4") << "\r\n";
					bool useIPv4 = AskYN();
					conf << "ipv4=" << (useIPv4 ? "true" : "false") << "\r\n";
				}
			}(conf, lang);
	}
	std::cout << "Config: " << std::endl;
	std::cout << conf.str() << std::endl;
	std::ofstream confFile("i2pd_.conf"); 
	confFile << conf.str();
	confFile.close();
}
#endif
