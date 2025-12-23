#include "DicProducer.h"

int main(int argc, char * argv[])
{
    //英文词典
    DicProducer producer("../raw_data/module1/yuliao/english.txt");
    producer.buildEnDict();
    producer.storeDict("../data/dicEn.dat");
    producer.createEnIndex();
    producer.storeIndex("../data/dicindexEn.dat");

    //中文词典
    DicProducer producer2("../raw_data/module1/yuliao/art");
    producer2.buildCnDict();
    producer2.storeDict("../data/dicCn.dat");
    producer2.createCnIndex();
    producer2.storeIndex("../data/dicindexCn.dat");

    return 0;
}
