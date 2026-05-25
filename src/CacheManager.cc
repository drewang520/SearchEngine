#include "CacheManager.h"
#include "Configuration.h"
#include "LRUCache.h"
#include "Logger.h"
#include "TimerManager.h"
#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <vector>

CacheManager& CacheManager::createCacheManger()
{
    static CacheManager cacheManager;
    return cacheManager;
}

CacheManager::CacheManager()
: m_cacheNum(std::stoi(Configuration::createpInstance().getConfig().at("threadNums")))
, m_writeCacheDelay(std::stoi(Configuration::createpInstance().getConfig().at("writeCachedelay")))
, m_cacheDataPath(Configuration::createpInstance().getConfig().at("cacheData"))
, m_caches()
, m_cacheMutexs(m_cacheNum)
{
    LRUCache& mainCache = getMainCache();
    mainCache.readFromFile(m_cacheDataPath);
    LOG_INFO("CacheManager initialized workerCaches=" + std::to_string(m_cacheNum)
             + " cacheData=" + m_cacheDataPath
             + " writeDelay=" + std::to_string(m_writeCacheDelay));
    m_caches.reserve(m_cacheNum);
    for (size_t idx = 0; idx < m_cacheNum; ++idx)
    {
        m_caches.push_back(LRUCache(mainCache));
    }
}

LRUCache & CacheManager::getCache(size_t cacheID)
{
    if (cacheID >= m_caches.size())
    {
        LOG_ERROR("CacheManager::getCache invalid cacheID=" + std::to_string(cacheID)
                  + " cacheNum=" + std::to_string(m_caches.size()));
        throw std::out_of_range("CacheManager::getCache invalid cacheID");
    }
    return m_caches[cacheID];
}

LRUCache& CacheManager::getMainCache()
{
    static LRUCache mainCache;
    return mainCache;
}

std::mutex & CacheManager::getMutex(size_t cacheID)
{
    if (cacheID >= m_cacheMutexs.size())
    {
        LOG_ERROR("CacheManager::getMutex invalid cacheID=" + std::to_string(cacheID)
                  + " mutexNum=" + std::to_string(m_cacheMutexs.size()));
        throw std::out_of_range("CacheManager::getMutex invalid cacheID");
    }
    return m_cacheMutexs[cacheID];
}

void CacheManager::PeriodicalUpdateCache()
{
    LRUCache& mainCache = getMainCache();
    size_t mergedUpdates = 0;
    {
        std::vector<std::unique_lock<std::mutex>> locks;
        locks.reserve(m_cacheMutexs.size());
        for (auto& mutex : m_cacheMutexs)
        {
            locks.emplace_back(mutex);
        }

        for (auto& cache : m_caches)
        {
            mergedUpdates += mergePendingUpdates(cache, mainCache);
        }
        mainCache.getPendingUpdateList().clear();

        for (auto& cache : m_caches)
        {
            cache.update(mainCache);
        }
    }
    LOG_INFO("cache sync finished workerCaches=" + std::to_string(m_cacheNum)
             + " mergedUpdates=" + std::to_string(mergedUpdates)
             + " mainCacheSize=" + std::to_string(mainCache.getResultList().size()));
    writeMainCacheIfNeeded(mainCache);
}

size_t CacheManager::mergePendingUpdates(LRUCache& cache, LRUCache& mainCache)
{
    auto & resultList = mainCache.getResultList(); 
    auto & hashmap = mainCache.getHashMap();
    auto & pendingUpdateList = cache.getPendingUpdateList();
    size_t mergedCount = pendingUpdateList.size();

    for (auto & [queryWord, value] : pendingUpdateList)
    {
        auto iter = hashmap.find(queryWord);            
        if (iter == hashmap.end())
        {
            mainCache.addElem(queryWord, value);
        }
        else 
        {
            resultList.splice(resultList.begin(), resultList, iter->second);
        }
    }

    pendingUpdateList.clear();
    return mergedCount;
}

void CacheManager::writeMainCacheIfNeeded(LRUCache& mainCache)
{
    TimerManager * timerManager = TimerManager::createTimerManager();
    if (timerManager->getWriteCacheTime() == m_writeCacheDelay)
    {
        LOG_INFO("write main cache to file path=" + m_cacheDataPath
                 + " elapsed=" + std::to_string(timerManager->getWriteCacheTime()));
        mainCache.writeToFile(m_cacheDataPath);
        timerManager->clearCacheTime();
    }
}
