#include "Configuration.h"
#include "Dictionary.h"

int main()
{
    Configuration * pInstance = Configuration::createpInstance("../config/config.json");
    Dictionary dic(pInstance);
    dic.getDict();
    dic.getIndex();
    return 0;
}
