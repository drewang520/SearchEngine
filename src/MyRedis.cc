#include "MyRedis.h"
#include "KeyRecommander.h"
#include <WebPageSearcher.h>
#include "ProtocolParser.h"
#include <sw/redis++/connection.h>
#include <iostream>

using namespace Protocol;

MyRedis::MyRedis(Redis & redis)
: _redis(std::move(redis))
{

}

string MyRedis::RedisTransaction(const string& queryWord, KeyRecommander& keyRecommander)
{
    long long retval = _redis.exists(queryWord);
    if (1 == retval)
    {
        std::cout << "queryWord is found at redis!" << "\n";
        /* return *_redis.get(queryWord); */    
        return _redis.get(queryWord).value();    

    }
    else 
    {
        std::cout << "queryword is no exists at redis" << "\n";
        string redis_val = ProtocolParser::JsonToString(
                             ProtocolParser::vecToJson(keyRecommander.doQuery()));
        _redis.set(queryWord, redis_val);
        return redis_val;
    }
}

string MyRedis::RedisTransaction(const string& queryWord, WebPageSearch & webPageSearch)
{
    long long retval = _redis.exists(queryWord);
    if (1 == retval)
    {
        std::cout << "queryWord is found at redis!" << "\n";
        /* return *_redis.get(queryWord); */    
        return _redis.get(queryWord).value();    

    }
    else 
    {
        std::cout << "queryword is no exists at redis" << "\n";
        string redis_val = ProtocolParser::JsonToString(
                                 ProtocolParser::vecWebToJson(webPageSearch.doQuery()));            
        _redis.set(queryWord, redis_val);
        return redis_val;
    }
}

