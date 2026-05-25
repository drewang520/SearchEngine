#include "pageLibPreprocessor.h"

int main(int argc, char * argv[])
{
    Configuration& config = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");
    pageLibPreprocessor page(config);
    page.buildInvertIndexMap();
    page.store();
}
