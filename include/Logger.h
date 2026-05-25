#ifndef LOGGER_H
#define LOGGER_H

#include <log4cpp/Category.hh>
#include <string>

class Logger
{
public:
    static void init(const std::string& configPath = "../config/log4cpp.properties");
    static void shutdown();
    static log4cpp::Category& root();
};

#define LOG_DEBUG(msg) Logger::root().debug(msg)
#define LOG_INFO(msg)  Logger::root().info(msg)
#define LOG_WARN(msg)  Logger::root().warn(msg)
#define LOG_ERROR(msg) Logger::root().error(msg)

#endif
