#include "pageLibPreprocessor.h"
#include "Configuration.h"

int main()
{
    Configuration::setConfigurFilePath("../config/config.json");
    Configuration * config = Configuration::createpInstance();
    pageLibPreprocessor page;
    page.buildInvertIndexMap(config->getConfig()["newripepage"], config->getConfig()["newoffset"]);
    page.store(config->getConfig()["invertIndexTable"]);
}
