#include "RecommandSearchServer.h"

int main(int argc, char * argv[])
{
    Configuration * config = Configuration::createpInstance();
    
    ConnectionOptions connection_options;
    connection_options.host = config->getConfig().at("host");
    connection_options.port = stoi(config->getConfig().at("redis_port"));
    Redis redis(connection_options);

    RecommandSearchServer RSserver(config, redis);
    RSserver.start();

    return 0;
}
