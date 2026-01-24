#include "RecommandSearchServer_Myredis.h"

int main(int argc, char * argv[])
{
    Configuration * config = Configuration::createpInstance();
    ConnectionOptions connection;
    connection.host = config->getConfig().at("host");
    connection.port = std::stoi(config->getConfig().at("redis_port"));
    Redis myredis(connection);
    RecommandSearchServer RSserver(config, myredis);
    RSserver.start();

    return 0;
}
