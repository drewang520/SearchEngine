#include "Logger.h"
#include <log4cpp/BasicConfigurator.hh>
#include <log4cpp/Configurator.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <atomic>
#include <mutex>

namespace
{
std::once_flag initFlag;
std::atomic_bool initialized{false};
}

void Logger::init(const std::string& configPath)
{
    std::call_once(initFlag, [&configPath]() {
        try
        {
            log4cpp::PropertyConfigurator::configure(configPath);
        }
        catch (const log4cpp::ConfigureFailure&)
        {
            log4cpp::BasicConfigurator::configure();
        }
        initialized.store(true);
    });
}

void Logger::shutdown()
{
    if (initialized.exchange(false))
    {
        log4cpp::Category::shutdown();
    }
}

log4cpp::Category& Logger::root()
{
    if (!initialized.load())
    {
        init();
    }
    return log4cpp::Category::getRoot();
}
