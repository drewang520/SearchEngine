#include "Configuration.h"
#include "PageProducer.h"

int main()
{
    Configuration * config = Configuration::createpInstance("../config/config.json");
    PageProducer page(config->getConfig()["page_src"]);

    /* page.store(config->getConfig()["ripepage"], config->getConfig()["pageoffset"]); */
    page.pageDeduplicat(config->getConfig()["newripepage"], config->getConfig()["newoffset"]);
    config->destory();
}
