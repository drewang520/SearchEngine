#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <map>
#include <set>

using std::set;
using std::string;
using std::map;

class Configuration
{
public:
    static void setConfigurFilePath(const string& config_path);
    static void setConfigurFilePath(const char * config_path);
    static Configuration * createpInstance();
    map<string, string>& getConfig();
    set<string>& getStopWords();

private:
    static void init();
    static void destory();

    Configuration(const string& config_path);
    Configuration(const Configuration& config) = delete;
    Configuration & operator=(const Configuration& config) = delete;
    Configuration(Configuration&& config) = delete;
    Configuration & operator=(Configuration&& config) = delete;
    ~Configuration() = default;

private:
    static Configuration * pInstance;
    static pthread_once_t once;
    static string _filepath;
    map<string, string> _config;
    set<string> _stopWords;
};

#endif
