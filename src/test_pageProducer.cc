#include "PageProducer.h"

int main(int argc, char * argv[])
{
    Configuration& config = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");
    PageProducer page(config);
    /* page.store(); */
    page.pageDeduplicat();
}
