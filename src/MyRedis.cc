#include "MyRedis.h"
#include "KeyRecommander.h"
#include <WebPageSearcher.h>
#include "ProtocolParser.h"
#include <iostream>
#include <mutex>

using namespace Protocol;

MyRedis::MyRedis(Redis & redis)
: m_redis(std::move(redis))
, m_mutex()
{

}

string MyRedis::RedisTransaction(const string& queryWord, KeyRecommander& keyRecommander)
{
    auto value = m_redis.get(queryWord);
    if (value)
    {
        std::cout << "queryWord is found at redis!" << "\n";
        /* return value.value(); */
        return *value;    
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    value = m_redis.get(queryWord);
    if (value)
    {
        std::cout << "redis has updated, queryWord is exist!" << "\n";
        /* return value.value(); */
        return *value;    
    }

    // 只有第一个进入的线程会执行查询操作
    std::cout << "queryword is no exists at redis" << "\n";
    string redis_val = ProtocolParser::JsonToString(
                         ProtocolParser::vecToJson(keyRecommander.doQuery()));
    m_redis.set(queryWord, redis_val);
    return redis_val;
}

string MyRedis::RedisTransaction(const string& queryWord, WebPageSearch & webPageSearch)
{
    if (m_redis.exists(queryWord))
    {
        std::cout << "queryWord is found at redis!" << "\n";
        /* return *m_redis.get(queryWord); */    
        return m_redis.get(queryWord).value();    
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_redis.exists(queryWord))
    {
        std::cout << "redis has updated, queryWord is exist!" << "\n";
        return m_redis.get(queryWord).value();
    }
    
    std::cout << "queryword is no exists at redis" << "\n";
    string redis_val = ProtocolParser::JsonToString(
                             ProtocolParser::vecWebToJson(webPageSearch.doQuery()));            
    m_redis.set(queryWord, redis_val);
    return redis_val;
}

void MyRedis::selectDb(unsigned int index)
{
    m_redis.command("select", index);
}

