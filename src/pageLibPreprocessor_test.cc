#include "pageLibPreprocessor.h"

int main()
{
    Configuration * config = Configuration::createpInstance();
    pageLibPreprocessor page(config);
    page.buildInvertIndexMap();
    page.store();
}
