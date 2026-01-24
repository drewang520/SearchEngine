#include "CacheManager.h"
#include "Configuration.h"
#include "LRUCache.h"
#include "TimerManager.h"
#include <cstddef>
#include <pthread.h>
#include <stdlib.h>
#include <mutex>
#include <iostream>

CacheManager * CacheManager::_pCacheManager = nullptr;
pthread_once_t  CacheManager::_pthreadInit = PTHREAD_ONCE_INIT;
LRUCache CacheManager::_mainCache = LRUCache();

CacheManager * CacheManager::createCacheManger()
{
    pthread_once(&_pthreadInit, init);
    return _pCacheManager;
}

void CacheManager::init()
{
    _pCacheManager = new CacheManager();
    atexit(destory);
}

CacheManager::CacheManager()
: _cacheNum(std::stoi(Configuration::createpInstance()->getConfig().at("threadNums")))
, _caches()
{
    // 锁未初始化会出大问题
     _cacheMutexs = std::vector<std::mutex>(_cacheNum + 1);
    _mainCache.readFromFile(Configuration::createpInstance()->getConfig().at("cacheData"));
    for (size_t idx = 0; idx < _cacheNum; ++idx)
    {
        _caches.push_back(LRUCache(_mainCache));
    }
}

LRUCache & CacheManager::getCache(size_t cacheID)
{
    return _caches[cacheID];
}

LRUCache& CacheManager::getMainCache()
{
    return _mainCache;
}

std::mutex & CacheManager::getMutex(size_t cacheID)
{
    return _cacheMutexs[cacheID];
}

void CacheManager::PeriodicalUpdateCache()
{
    auto & resultList = _mainCache.getResultList(); 
    auto & hashmap = _mainCache.getHashMap();
    for (size_t cacheid = 0; cacheid < _cacheNum; ++cacheid)
    {
        std::unique_lock<std::mutex> lock(_cacheMutexs[cacheid], std::try_to_lock);
        if (lock.owns_lock())
        {
            auto & workPendingUpdateList = getCache(cacheid).getPendingUpdateList();
            for (auto & [queryWord, value] : workPendingUpdateList)
            {
                auto iter = hashmap.find(queryWord);            
                if (iter == hashmap.end())
                {
                    _mainCache.addElem(queryWord, value);
                }
                else 
                {
                   resultList.splice(resultList.begin(), resultList, iter->second);
                }
            }
            workPendingUpdateList.clear();
        }
        else 
        {
            std::cout << "出现了竞争条件" << "\n";
            continue;
        }
    }
    {
        size_t cacheID = 0;
        std::lock_guard<std::mutex> lock0(_cacheMutexs[cacheID++]);
        std::lock_guard<std::mutex> lock1(_cacheMutexs[cacheID++]); 
        std::lock_guard<std::mutex> lock2(_cacheMutexs[cacheID++]); 
        std::lock_guard<std::mutex> lock3(_cacheMutexs[cacheID++]); 
        std::lock_guard<std::mutex> lock4(_cacheMutexs[cacheID++]); 
        for (auto & cache : _caches)
        {
            cache.update(_mainCache);
        }
    }

    TimerManager * timerManager = TimerManager::createTimerManager();
    if (timerManager->getWriteCacheTime() == std::stoi(Configuration::createpInstance()->getConfig().at("writeCachedelay")))
    {
        std::cout << "time : " << timerManager->getWriteCacheTime() << "\n";
        _mainCache.writeToFile(Configuration::createpInstance()->getConfig().at("cacheData"));
        timerManager->clearCacheTime();
    }
        /* std::cout << "没有锁上" << "\n"; */
}


void CacheManager::destory()
{
    if (_pCacheManager)
    {
        delete _pCacheManager;
        _pCacheManager = nullptr;
    }
}

