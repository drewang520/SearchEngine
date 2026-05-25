#include <sw/redis++/redis++.h>
#include <mutex>
#include <string>
#include <vector>

class KeyRecommander;
class WebPageSearch;
class WebPage;

using std::string;
using std::vector;
using namespace sw::redis;


class MyRedis
{
public:
    MyRedis(Redis & redis);
    string RedisTransaction(const string& queryWord, KeyRecommander& keyRecommander);
    string RedisTransaction(const string& queryWord, WebPageSearch & webPageSearch); 
    void selectDb(unsigned int index);

private:
    Redis m_redis;
    std::mutex m_mutex;
};
