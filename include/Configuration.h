#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <string>
#include <map>
#include <set>

using std::set;
using std::string;
using std::map;

class Configuration
{
public:
    static Configuration * createpInstance(const string& config_path);

    static void destory();

    map<string, string>& getConfig();

private:
    Configuration(const string& config_path);
    Configuration(const Configuration& config) = delete;
    Configuration & operator=(const Configuration& config) = delete;
    Configuration(Configuration&& config) = delete;
    Configuration & operator=(Configuration&& config) = delete;
    ~Configuration() = default;

private:
    static Configuration * pInstance;
    map<string, string> _config;
    set<string> _stopWords;
};

#endif
