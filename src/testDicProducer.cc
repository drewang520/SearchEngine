#include "Configuration.h"
#include "DicProducer.h"

int main(int argc, char * argv[])
{
    Configuration * pInstance = Configuration::createpInstance("../config/config.json");
    map<string, string> config = pInstance->getConfig();
    //英文词典
    DicProducer producer(config["En_dict"]);
    producer.buildEnDict(config["stop_words_en"]);
    producer.storeDict(config["dicEn.dat"]);
    producer.createEnIndex();
    producer.storeIndex(config["dicindexEn.dat"]);

    //中文词典
    DicProducer producer2(config["Cn_dict"]);
    producer2.buildCnDict(config["stop_words_cn"]);
    producer2.storeDict(config["dicCn.dat"]);
    producer2.createCnIndex();
    producer2.storeIndex(config["dicindexCn.dat"]);

    return 0;
}
