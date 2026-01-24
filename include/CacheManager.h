#ifndef _CACHE_MANAGER_H
#define _CACHE_MANAGER_H

#include "LRUCache.h"
#include <mutex>
#include <pthread.h>
#include <vector>
#include <string>

class LRUCache;
using std::vector;
using std::string;

class CacheManager
{
public:
    static CacheManager * createCacheManger(); 

    static void init();
    LRUCache & getCache(size_t);
    std::mutex & getMutex(size_t);
    void PeriodicalUpdateCache();
    static LRUCache& getMainCache();

private:
    CacheManager();
    CacheManager(const CacheManager& cacheManager) = delete;
    CacheManager & operator=(const CacheManager& cacheManager) = delete;
    CacheManager(CacheManager&& cacheManager) = delete;
    CacheManager& operator=(CacheManager&& cacheManager) = delete;
    ~CacheManager() = default;

    static void destory();

private:
    vector<LRUCache> _caches;    
    vector<std::mutex> _cacheMutexs;
    size_t _cacheNum;

    static LRUCache _mainCache;
    static std::mutex _mutex;
    static CacheManager * _pCacheManager;
    static pthread_once_t  _pthreadInit;
};


#endif
