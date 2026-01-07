#include "Configuration.h"
#include "Dictionary.h"

int main()
{
    Configuration * pInstance = Configuration::createpInstance();
    Dictionary dic(pInstance);
    dic.getDict();
    dic.getIndex();
    return 0;
}
