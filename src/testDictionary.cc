#include "Configuration.h"
#include "Dictionary.h"

int main()
{
    Configuration::setConfigurFilePath("../config/config.json");
    Configuration * pInstance = Configuration::createpInstance();
    Dictionary dic(pInstance);
    dic.getDict();
    dic.getIndex();
    return 0;
}
