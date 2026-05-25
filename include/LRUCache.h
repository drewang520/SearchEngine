#ifndef _LRUCACHE_H
#define _LRUCACHE_H

#include "ProtocolParser.h"
#include <unordered_map>
#include <list>
#include <utility>
#include <string>

class KeyRecommander;
using namespace nlohmann;
using namespace Protocol;

using std::unordered_map;
using std::pair;
using std::list;

class Configuration;

class LRUCache
{
public:
    LRUCache();
    LRUCache(const LRUCache& cache);

    string CacheTransaction(const string& queryWord, KeyRecommander& keyRecommander);
    void addElem(const string& queryWord, const json& value);
    bool get(const string& queryWord, json & value);
    void readFromFile(const string& filename);
    void writeToFile(const string& filename);
    void update(const LRUCache& cache);
    list<pair<std::string, json>>& getPendingUpdateList();
    
    void getCacheElem() const;
    list<pair<std::string, json>>& getResultList();
    unordered_map<std::string, list<pair<std::string, json>>::iterator>& getHashMap();

private:
    size_t m_capacity;
    unordered_map<std::string, list<pair<std::string, json>>::iterator> m_hashMap;
    list<pair<std::string, json>> m_resultList;
    list<pair<std::string, json>> m_pendingUpdateList;
};



#endif
