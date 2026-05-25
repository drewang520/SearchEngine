#include "LRUCache.h"
#include "Configuration.h"
#include "KeyRecommander.h"
#include "ProtocolParser.h"

#include <iostream>
#include <fstream>
#include <sstream>

LRUCache::LRUCache()
: m_capacity(std::stoi(Configuration::createpInstance().getConfig().at("cacheSize")))
, m_hashMap()
, m_resultList()
, m_pendingUpdateList()
{

}

LRUCache::LRUCache(const LRUCache& cache)
: m_capacity(cache.m_capacity)
, m_hashMap()
, m_resultList()
, m_pendingUpdateList()
{
    update(cache); 
}

// 只用一个全局Cache版本，粒度很大
string LRUCache::CacheTransaction(const string& queryWord, KeyRecommander& keyRecommander)
{
    auto iter = m_hashMap.find(queryWord);
    if (iter != m_hashMap.end())
    {
        m_resultList.splice(m_resultList.begin(), m_resultList, iter->second);
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
    if (m_resultList.size() == m_capacity)
    {
        // 执行LRU算法
        m_hashMap.erase(m_resultList.back().first);
        m_resultList.pop_back();
    }
    m_resultList.push_front(std::move(std::make_pair(queryWord, value)));
    m_hashMap[queryWord] = m_resultList.begin();
    m_pendingUpdateList.push_front(std::move(std::make_pair(queryWord, value)));
}

bool LRUCache::get(const string& queryWord, json& value)
{
    auto iter = m_hashMap.find(queryWord);
    if (iter != m_hashMap.end())
    {
        m_resultList.splice(m_resultList.begin(), m_resultList, iter->second);
        m_pendingUpdateList.remove(*(iter->second));
        m_pendingUpdateList.push_front(std::move(std::make_pair(iter->second->first, iter->second->second)));
        value = iter->second->second;
        return true;
    }
    return false;
}

list<pair<std::string, json>>& LRUCache::getPendingUpdateList()
{
    return m_pendingUpdateList;
}

void LRUCache::update(const LRUCache& cache)
{
    // 出现问题的地方，折腾了很久
    /* m_resultList.clear(); */
    /* m_hashMap.clear(); */
    /* m_resultList = cache.m_resultList; */
    /* m_hashMap = cache.m_hashMap; //这里会出现迭代器相关的问题 */

    m_resultList = cache.m_resultList;
    m_hashMap.clear();
    for (auto iter = m_resultList.begin(); iter != m_resultList.end(); ++iter)
    {
        m_hashMap[iter->first] = iter;
    }
}

void LRUCache::getCacheElem() const
{
    std::cout << "cache.size: " << m_resultList.size() << "\n";
    for (auto it = m_resultList.begin(); it != m_resultList.end(); ++it)
    {
        std::cout << it->first << ":" << it->second << "\n"; 
    }
    /* for (const auto & [word, value] : m_resultList) */
    /* { */
    /*     std::cout << word << ": " << value << "\n"; */
    /* } */
}

list<pair<std::string, json>>& LRUCache::getResultList()
{
    return m_resultList;
}

unordered_map<std::string, list<pair<std::string, json>>::iterator>& LRUCache::getHashMap()
{
    return m_hashMap;
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
        m_resultList.push_back(std::make_pair(word, value)); 
        m_hashMap[word] = --m_resultList.end();
    }
}

void LRUCache::writeToFile(const string& filename)
{
    std::ofstream ofs(filename);
    for (const auto & [word, value] : m_resultList)
    {
        ofs << word << " " << value << "\n";
    }
}
