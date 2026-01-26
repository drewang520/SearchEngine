#include "RecommandSearchServer_MyCache2.h"

int main(int argc, char * argv[])
{
    Configuration * config = Configuration::createpInstance();
    RecommandSearchServer RSserver(config);
    RSserver.start();

    return 0;
}
