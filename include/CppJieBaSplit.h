#ifndef _CPPJIEBA_SPLIT_H
#define _CPPJIEBA_SPLIT_H

#include "Configuration.h"
#include "NoCopyable.h"
#include "cppjieba/Jieba.hpp"
#include <vector>
#include <string>

using std::string;
using std::vector;

class CppJiebaSplit
: public NoCopyable
{
public:
    CppJiebaSplit()
    {
        Configuration::setConfigurFilePath("../config/config.json");
        _config = Configuration::createpInstance();

        const char * dict_path = _config->getConfig()["dict_path"].c_str();
        const char * model_path = _config->getConfig()["model_path"].c_str();
        const char * user_dict_path = _config->getConfig()["user_dict_path"].c_str();
        const char * idf_path = _config->getConfig()["idf_path"].c_str();
        const char * stop_word_path = _config->getConfig()["stop_word_path"].c_str();
        cppjieba::Jieba jieba(dict_path, model_path, user_dict_path, idf_path, stop_word_path);
        _jieba = &jieba;
    }

    vector<string> cut(const string& key)
    {
        vector<string> Words; 
        _jieba->Cut(key, Words, true);
        return Words;
    }

private:
    Configuration * _config;
    cppjieba::Jieba * _jieba;
};

#endif
