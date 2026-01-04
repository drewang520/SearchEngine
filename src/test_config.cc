#include "Configuration.h"
#include <string>
#include <iostream>

int main()
{
    Configuration::setConfigurFilePath("../config/config.json");
    Configuration * pConfig = Configuration::createpInstance();

    std::map<std::string, string> _config = pConfig->getConfig();
    std::cout << "port: " << _config["port"] << "\n";
    std::cout << "En_dict: " << _config["En_dict"] << "\n";

    std::set<std::string> stop_words = pConfig->getStopWords();

}
