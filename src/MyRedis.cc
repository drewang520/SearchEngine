#include "MyRedis.h"
#include "KeyRecommander.h"
#include <WebPageSearcher.h>
#include "ProtocolParser.h"
#include <iostream>
#include <mutex>

using namespace Protocol;

MyRedis::MyRedis(Redis & redis)
: _redis(std::move(redis))
, _mutex()
{

}

string MyRedis::RedisTransaction(const string& queryWord, KeyRecommander& keyRecommander)
{
    auto value = _redis.get(queryWord);
    if (value)
    {
        std::cout << "queryWord is found at redis!" << "\n";
        /* return value.value(); */
        return *value;    
    }

    std::lock_guard<std::mutex> lock(_mutex);
    value = _redis.get(queryWord);
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
    _redis.set(queryWord, redis_val);
    return redis_val;
}

string MyRedis::RedisTransaction(const string& queryWord, WebPageSearch & webPageSearch)
{
    if (_redis.exists(queryWord))
    {
        std::cout << "queryWord is found at redis!" << "\n";
        /* return *_redis.get(queryWord); */    
        return _redis.get(queryWord).value();    
    }

    std::lock_guard<std::mutex> lock(_mutex);
    if (_redis.exists(queryWord))
    {
        std::cout << "redis has updated, queryWord is exist!" << "\n";
        return _redis.get(queryWord).value();
    }
    
    std::cout << "queryword is no exists at redis" << "\n";
    string redis_val = ProtocolParser::JsonToString(
                             ProtocolParser::vecWebToJson(webPageSearch.doQuery()));            
    _redis.set(queryWord, redis_val);
    return redis_val;
}

void MyRedis::selectDb(unsigned int index)
{
    _redis.command("select", index);
}

