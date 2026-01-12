#include "Configuration.h"
#include "KeyRecommander.h"
#include <string>
#include <iostream>

int main()
{
    Configuration * pConfig = Configuration::createpInstance();

    std::map<std::string, string> _config = pConfig->getConfig();
    std::cout << "ip: " << _config.at("ip") << "\n";
    std::cout << "En_dict: " << _config.at("En_dict") << "\n";

    KeyRecommander keyrecommander("喜欢你", pConfig);
    vector<string> commanderWords = keyrecommander.doQuery();
    for (auto word : commanderWords)
    {
        std::cout << "word: " << word << "\n";
    }
}
