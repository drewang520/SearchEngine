#include "PageProducer.h"

int main()
{
    Configuration * config = Configuration::createpInstance();
    PageProducer page(config);
    /* page.store(); */
    page.pageDeduplicat();
}
