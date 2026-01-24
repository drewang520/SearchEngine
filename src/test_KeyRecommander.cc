#include "Configuration.h"
#include "KeyRecommander.h"
#include <string>
#include <iostream>

int main()
{
    Configuration * pConfig = Configuration::createpInstance();

    std::map<std::string, string> _config = pConfig->getConfig();

    KeyRecommander keyrecommander("喜欢你", pConfig);
    vector<string> commanderWords = keyrecommander.doQuery();
    for (auto word : commanderWords)
    {
        std::cout << "word: " << word << "\n";
    }
}
