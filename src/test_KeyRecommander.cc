#include "KeyRecommander.h"
#include <string>
#include <iostream>

int main()
{
    Configuration * pConfig = Configuration::createpInstance("../config/config.json");

    std::map<std::string, string> _config = pConfig->getConfig();
    std::cout << "ip: " << _config["ip"] << "\n";
    std::cout << "En_dict: " << _config["En_dict"] << "\n";

    KeyRecommander keyrecommander("hello", nullptr,  pConfig);
    vector<string> commanderWords = keyrecommander.doQuery();
    for (auto word : commanderWords)
    {
        std::cout << "word: " << word << "\n";
    }

    pConfig->destory();
}
