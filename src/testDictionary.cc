#include "Configuration.h"
#include "Dictionary.h"

int main(int argc, char * argv[])
{
    Configuration& pInstance = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");
    Dictionary dic(pInstance);
    /* dic.getDict(); */
    /* dic.getIndex(); */
    return 0;
}
