#include "Configuration.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace
{
std::string jsonValueToString(const nlohmann::json& value)
{
    if (value.is_string())
    {
        return value.get<std::string>();
    }
    if (value.is_number_integer())
    {
        return std::to_string(value.get<int>());
    }
    if (value.is_number_unsigned())
    {
        return std::to_string(value.get<unsigned int>());
    }
    if (value.is_number_float())
    {
        return std::to_string(value.get<double>());
    }

    throw std::runtime_error("unsupported config value type: " + value.dump());
}

void loadStopWordsFromFile(const std::string& filename, std::set<std::string>& stopWords)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        throw std::runtime_error("failed to open stop words file: " + filename);
    }

    std::string line;
    while (std::getline(ifs, line))
    {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (!line.empty())
        {
            stopWords.insert(line);
        }
    }
}
}

Configuration::Configuration(const std::string& config_path)
{
    std::ifstream ifs(config_path);
    if (!ifs)
    {
        throw std::runtime_error("failed to open config file: " + config_path);
    }

    nlohmann::json file;
    ifs >> file;
    if (!file.is_object())
    {
        throw std::runtime_error("config root must be a JSON object: " + config_path);
    }

    for (const auto &[key, value]: file.items())
    {
        m_config[key] = jsonValueToString(value);
    }
    loadStopWords();
}

const std::map<std::string, std::string>& Configuration::getConfig() const
{
    return m_config;
}

void Configuration::loadStopWords()
{
    const char* stopWordsConfigKeys[] = {
        "stop_words_en",
        "stop_words_cn",
        "stop_word_path",
    };

    for (const char* key : stopWordsConfigKeys)
    {
        loadStopWordsFromFile(m_config.at(key), m_stopWords);
    }
}

const std::set<std::string>& Configuration::getStopWords() const
{
    return m_stopWords;
}

Configuration& Configuration::createpInstance(const std::string& config_path)
{
    static Configuration instance(config_path);
    return instance;
}
