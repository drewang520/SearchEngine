#include "Configuration.h"
#include <string>
#include <iostream>

int main(int argc, char * argv[])
{
    Configuration& pConfig = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");

    std::map<std::string, std::string> _config = pConfig.getConfig();
    std::cout << "port: " << _config["port"] << "\n";
    std::cout << "En_dict: " << _config["En_dict"] << "\n";

    std::set<std::string> stop_words = pConfig.getStopWords();

}
