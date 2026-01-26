#include "LRUCache.h"
#include "Configuration.h"
#include "KeyRecommander.h"
#include "ProtocolParser.h"

#include <iostream>
#include <fstream>
#include <sstream>

LRUCache::LRUCache()
: _capacity(std::stoi(Configuration::createpInstance()->getConfig().at("cacheSize")))
, _hashMap()
, _resultList()
, _pendingUpdateList()
{

}

LRUCache::LRUCache(const LRUCache& cache)
: _capacity(cache._capacity)
, _hashMap(cache._hashMap)
, _resultList(cache._resultList)
, _pendingUpdateList()
{
    
}

// 只用一个全局Cache版本，粒度很大
string LRUCache::CacheTransaction(const string& queryWord, KeyRecommander& keyRecommander)
{
    auto iter = _hashMap.find(queryWord);
    if (iter != _hashMap.end())
    {
        _resultList.splice(_resultList.begin(), _resultList, iter->second);
        return ProtocolParser::JsonToString(iter->second->second);
    }
    else 
    {
        auto value = ProtocolParser::vecToJson(keyRecommander.doQuery());
        addElem(queryWord, value);
        return ProtocolParser::JsonToString(value);
    }
}

void LRUCache::addElem(const string& queryWord, const json& value)
{
    if (_resultList.size() == _capacity)
    {
        // 执行LRU算法
        _hashMap.erase(_resultList.back().first);
        _resultList.pop_back();
    }
    _resultList.push_front(std::move(std::make_pair(queryWord, value)));
    _hashMap[queryWord] = _resultList.begin();
    _pendingUpdateList.push_front(std::move(std::make_pair(queryWord, value)));
}

bool LRUCache::get(const string& queryWord, json& value)
{
    auto iter = _hashMap.find(queryWord);
    if (iter != _hashMap.end())
    {
        _resultList.splice(_resultList.begin(), _resultList, iter->second);
        _pendingUpdateList.remove(*(iter->second));
        _pendingUpdateList.push_front(std::move(std::make_pair(iter->second->first, iter->second->second)));
        value = iter->second->second;
        return true;
    }
    return false;
}

list<pair<std::string, json>>& LRUCache::getPendingUpdateList()
{
    return _pendingUpdateList;
}

void LRUCache::update(const LRUCache& cache)
{
    // 出现问题的地方，折腾了很久
    /* _resultList.clear(); */
    /* _hashMap.clear(); */
    /* _resultList = cache._resultList; */
    /* _hashMap = cache._hashMap; //这里会出现迭代器相关的问题 */

    _resultList = cache._resultList;
    _hashMap.clear();
    for (auto iter = _resultList.begin(); iter != _resultList.end(); ++iter)
    {
        _hashMap[iter->first] = iter;
    }
}

void LRUCache::getCacheElem() const
{
    std::cout << "cache.size: " << _resultList.size() << "\n";
    for (auto it = _resultList.begin(); it != _resultList.end(); ++it)
    {
        std::cout << it->first << ":" << it->second << "\n"; 
    }
    /* for (const auto & [word, value] : _resultList) */
    /* { */
    /*     std::cout << word << ": " << value << "\n"; */
    /* } */
}

list<pair<std::string, json>>& LRUCache::getResultList()
{
    return _resultList;
}

unordered_map<std::string, list<pair<std::string, json>>::iterator>& LRUCache::getHashMap()
{
    return _hashMap;
}

void LRUCache::readFromFile(const string& filename)
{
    std::ifstream ifs(filename);
    string line;
    while (std::getline(ifs, line))
    {
        string word;
        json value;
        std::istringstream iss(line);    
        iss >> word >> value;
        _resultList.push_back(std::make_pair(word, value)); 
        _hashMap[word] = --_resultList.end();
    }
}

void LRUCache::writeToFile(const string& filename)
{
    std::ofstream ofs(filename);
    for (const auto & [word, value] : _resultList)
    {
        ofs << word << " " << value << "\n";
    }
}

