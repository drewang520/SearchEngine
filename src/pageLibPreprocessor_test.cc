#include "pageLibPreprocessor.h"
#include "Configuration.h"

int main()
{
    Configuration * config = Configuration::createpInstance("../config/config.json");
    pageLibPreprocessor page;
    page.buildInvertIndexMap(config->getConfig()["newripepage"], config->getConfig()["newoffset"]);

    config->destory();
}
