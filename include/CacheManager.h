#ifndef _CACHE_MANAGER_H
#define _CACHE_MANAGER_H

#include "LRUCache.h"
#include <mutex>
#include <vector>
#include <string>

class LRUCache;

class CacheManager
{
public:
    static CacheManager& createCacheManger(); 

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

    size_t mergePendingUpdates(LRUCache& cache, LRUCache& mainCache);
    void writeMainCacheIfNeeded(LRUCache& mainCache);

private:
    size_t m_cacheNum;
    int m_writeCacheDelay;
    std::string m_cacheDataPath;
    std::vector<LRUCache> m_caches;    
    std::vector<std::mutex> m_cacheMutexs;
};


#endif
