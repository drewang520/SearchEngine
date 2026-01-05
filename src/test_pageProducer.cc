#include "Configuration.h"
#include "PageProducer.h"

int main()
{
    Configuration::setConfigurFilePath("../config/config.json");
    Configuration * config = Configuration::createpInstance();
    PageProducer page(config->getConfig()["page_src"]);

    /* page.store(config->getConfig()["ripepage"], config->getConfig()["pageoffset"]); */
    page.pageDeduplicat(config->getConfig()["newripepage"], config->getConfig()["newoffset"]);
}
