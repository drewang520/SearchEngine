#include "DicProducer.h"

int main(int argc, char * argv[])
{
    DicProducer producer("../raw_data/module1/yuliao/english.txt");
    producer.buildEnDict();
    producer.storeDict("../data/dic.dat");
    /* producer.printFile(); */

    producer.createIndex();
    producer.storeIndex("../data/dicindex.dat");
    return 0;
}
