#include "RecommandSearchServer_MyCache2.h"
#include "Logger.h"

int main(int argc, char * argv[])
{
    Logger::init("../config/log4cpp.properties");
    {
        Configuration& config = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");
        RecommandSearchServer RSserver(config);
        RSserver.start();
    }
    
    Logger::shutdown();
    return 0;
}
