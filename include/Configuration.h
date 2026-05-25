#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <string>
#include <map>
#include <set>


class Configuration
{
public:
    static Configuration& createpInstance(const std::string& config_path = "../config/config.json");
    const std::map<std::string, std::string>& getConfig() const;
    const std::set<std::string>& getStopWords() const;
   
private:
    void loadStopWords();

    Configuration(const std::string& config_path);

    Configuration(const Configuration& config) = delete;
    Configuration & operator=(const Configuration& config) = delete;
    Configuration(Configuration&& config) = delete;
    Configuration & operator=(Configuration&& config) = delete;

    ~Configuration() = default;

    std::map<std::string, std::string> m_config;
    std::set<std::string> m_stopWords;
};

#endif
