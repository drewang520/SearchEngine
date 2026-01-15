#include "Configuration.h"
#include "nlohmann/json.hpp"
#include <pthread.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>

using std::ifstream;
using json = nlohmann::json;

Configuration * Configuration::pInstance = nullptr;
pthread_once_t Configuration::once = PTHREAD_ONCE_INIT;
string Configuration::_filepath = "../config/config.json";

Configuration::Configuration(const string& config_path)
{
    auto file_size = std::filesystem::file_size(config_path);
    ifstream ifs(config_path);
    char * buf = new char[file_size]();
    ifs.read(buf, file_size);
    json file = json::parse(buf);    
    for (const auto &[key, value]: file.items())
    {
        if (value.is_string())
        {
            _config[key] = value.get<std::string>();
        }
        else if (value.is_number())
        {
            _config[key] = std::to_string(value.get<int>());
        }
    }
    delete [] buf;
    LoadStopWords();
}

const map<string, string>& Configuration::getConfig() const
{
    return _config;
}

//使用对象直接获取配置文件value，不必先得到_config，方便
//但是单例提供的是指针，感觉没什么用
const string& Configuration::operator[](const string& key)
{
    return _config.at(key);
}

void Configuration::LoadStopWords()
{
    ifstream ifs1(_config.at("stop_words_en"));
    ifstream ifs2(_config.at("stop_words_cn"));
    ifstream ifs3(_config.at("stop_word_path"));
    string line;
    while (std::getline(ifs1, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        _stopWords.insert(line);
    }
    while (std::getline(ifs2, line))
    {
        // 删除行内所有的\r
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        _stopWords.insert(line);
    }
    /* std::cout << "_stopWords.size = " << _stopWords.size() << "\n"; */
    while (std::getline(ifs3, line))
    {
        // 删除行内所有的\r
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        _stopWords.insert(line);
    }
    /* std::cout << "_stopWords.size = " << _stopWords.size() << "\n"; */
}

const set<string>& Configuration::getStopWords() const
{
    return _stopWords;
}

Configuration * Configuration::createpInstance()
{
    pthread_once(&once, init);
    return pInstance;
}

void Configuration::init()
{
    pInstance = new Configuration(_filepath);
    atexit(destory);
}

void Configuration::destory()
{
    if (pInstance)
    {
        delete pInstance;
        pInstance = nullptr;
    }
}

