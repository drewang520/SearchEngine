#include "Configuration.h"
#include "KeyRecommander.h"
#include <string>
#include <iostream>

int main(int argc, char * argv[])
{
    Configuration& pConfig = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");

    std::string query = argc > 2 ? argv[2] : "喜欢你";
    KeyRecommander keyrecommander(query, pConfig);
    vector<string> commanderWords = keyrecommander.doQuery();
    for (auto word : commanderWords)
    {
        std::cout << "word: " << word << "\n";
    }
}
