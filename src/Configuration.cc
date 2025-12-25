#include "Configuration.h"
#include "nlohmann/json.hpp"

#include <fstream>

using std::ifstream;

using json = nlohmann::json;

Configuration * Configuration::pInstance = nullptr;

Configuration::Configuration(const string& config_path)
{
    ifstream ifs(config_path);
    char * buf = new char[4096]();
    ifs.read(buf, 4096);
    json file = json::parse(buf);    
    for (auto [key, value]: file.items())
    {
        _config.insert({key, value});
    }
    
    delete [] buf;
}

map<string, string>& Configuration::getConfig()
{
    return pInstance->_config;
}


Configuration * Configuration::createpInstance(const string& config_path)
{
    if (pInstance == nullptr)
    {
        pInstance = new Configuration(config_path);
    }
    return pInstance;
}

void Configuration::destory()
{
    if (pInstance)
    {
        delete pInstance;
    }
}
