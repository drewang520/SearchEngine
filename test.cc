#include <cstdint>
#include <iostream>
#include <simhash/Simhasher.hpp>
#include <string>
#include <utility>
#include <vector>

using namespace simhash;
void test2()
{
    Simhasher simhasher("./raw_data/module1/dict/jieba.dict.utf8", 
                        "./raw_data/module1/dict/hmm_model.utf8", 
                        "./raw_data/module1/dict/idf.utf8", 
                        "./raw_data/module1/dict/stop_words.utf8");
    const char * bin1 = "100010110110";
    const char * bin2 = "110001110011";
    uint64_t u1 = Simhasher::binaryStringToUint64(bin1);
    uint64_t u2 = Simhasher::binaryStringToUint64(bin2);
    std::cout << "默认海明距离为3, isEqual = " << (Simhasher::isEqual(u1, u2)) << "\n";
    std::cout << "默认海明距离为5, isEqual = " << (Simhasher::isEqual(u1, u2, 5)) << "\n";
}
void test()
{
    std::vector<int> vec;
    vec.reserve(5);
    std::cout << "vec.size = " << vec.capacity() << "\n";
}
int main()
{
    int id = 0;
    std::string str("drewang");
    auto && rvaue = std::make_pair(id, str);
    const auto & rvalue = std::make_pair(id + 1, str);
    test();
    test2();
    return 0;
}
