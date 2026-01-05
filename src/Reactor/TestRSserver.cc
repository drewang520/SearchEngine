#include "RecommandSearchServer.h"

int main(int argc, char * argv[])
{
    Configuration::setConfigurFilePath("../config/config.json");
    Configuration * config = Configuration::createpInstance();
    RecommandSearchServer RSserver(config);
    RSserver.start();

    return 0;
}
