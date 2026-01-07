#include "pageLibPreprocessor.h"
#include "Configuration.h"

int main()
{
    Configuration * config = Configuration::createpInstance();
    pageLibPreprocessor page;
    page.buildInvertIndexMap(config->getConfig()["newripepage"], config->getConfig()["newoffset"]);
    page.store(config->getConfig()["invertIndexTable"]);
}
