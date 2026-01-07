#include "Configuration.h"
#include "DicProducer.h"

int main(int argc, char * argv[])
{
    Configuration * pInstance = Configuration::createpInstance();
    map<string, string> config = pInstance->getConfig();
    //英文词典
    DicProducer producer(config["En_dict"], pInstance);
    producer.buildEnDict(pInstance->getStopWords());
    producer.storeDict(config["dicEn.dat"]);
    producer.createEnIndex();
    producer.storeIndex(config["dicindexEn.dat"]);

    //中文词典
    DicProducer producer2(config["Cn_dict"], pInstance);
    producer2.buildCnDict(pInstance->getStopWords());
    producer2.storeDict(config["dicCn.dat"]);
    producer2.createCnIndex();
    producer2.storeIndex(config["dicindexCn.dat"]);

    return 0;
}
