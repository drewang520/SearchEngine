#include "RecommandSearchServer_MyCache.h"

int main(int argc, char * argv[])
{
    Configuration& config = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");
    
    RecommandSearchServer RSserver(config);
    RSserver.start();

    return 0;
}
