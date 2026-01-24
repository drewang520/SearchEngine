#include "TimerManager.h"
#include "CacheManager.h"
#include "Configuration.h"
#include "LRUCache.h"
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <poll.h>
#include <ctime>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <stdlib.h>
#include <iostream>

TimerManager * TimerManager::_timerManager = nullptr;
pthread_once_t TimerManager::once_init = PTHREAD_ONCE_INIT;

TimerManager * TimerManager::createTimerManager()
{
    pthread_once(&once_init, init);
    return _timerManager;
}

void TimerManager::destory()
{
    if (_timerManager)
    {
        delete _timerManager;
        _timerManager = nullptr;
    }
}

void TimerManager::init()
{
    _timerManager = new TimerManager();
    atexit(destory);
}

TimerManager::TimerManager()
: _timefd(createTimerfd())
, _writefd(createTimerfd())
, _isStarted(false)
, _writeCacheTime(0)
, _delay(std::stoi(Configuration::createpInstance()->getConfig().at("delay")))
, _interval(std::stoi(Configuration::createpInstance()->getConfig().at("interval")))
, _writeCachedelay(std::stoi(Configuration::createpInstance()->getConfig().at("writeCachedelay")))
, _writeCacheinterval(std::stoi(Configuration::createpInstance()->getConfig().at("writeCacheinterval")))
{
    _timerCallbacks[_timefd] = std::vector<TimerCallback>();
    _timerCallbacks[_writefd] = std::vector<TimerCallback>();
}

int TimerManager::createTimerfd()
{
    int timerfd = timerfd_create(CLOCK_REALTIME, 0);    
    if (-1 == timerfd)
    {
        perror("timerfd create");
    }
    return timerfd;
}

int TimerManager::getWriteCacheTime()
{
    return _writeCacheTime;
}

void TimerManager::clearCacheTime()
{
    _writeCacheTime = 0;
}

void TimerManager::setTimerfd(int fd, int delay, int interval)
{
    struct itimerspec itimer;
    itimer.it_value.tv_sec = delay;
    itimer.it_value.tv_nsec = 0;
    itimer.it_interval.tv_sec = interval;
    itimer.it_interval.tv_nsec = 0;
    int ret = timerfd_settime(fd, 0, &itimer, nullptr);
    if (-1 == ret)
    {
        perror("timerfd settime");
    }
}

void TimerManager::start()
{
    struct pollfd fds;       
    fds.fd = _timefd;
    fds.events = POLLIN;
    /* fds[0].fd = _timefd; */
    /* fds[0].events = POLLIN; */
    
    /* fds[1].fd = _writefd; */
    /* fds[1].events = POLLIN; */
    CacheUpdateTask cacheUpdatetask;
    /* WriteCacheTask writeCacheTask; */
    _timerManager->attach(&cacheUpdatetask);
    /* _timerManager->attach(&writeCacheTask); */
    /* registerTimer(_timefd, std::bind(&CacheManager::PeriodicalUpdateCache, CacheManager::createCacheManger())); */
    /* registerTimer(_writefd, std::bind(&LRUCache::writeToFile, CacheManager::getMainCache(), */ 
    /*                                              Configuration::createpInstance()->getConfig().at("cacheData"))); */
    setTimerfd(_timefd, _delay, _interval);
    /* setTimerfd(fds[1].fd, _writeCachedelay, _writeCacheinterval); */
    
    _isStarted = true;
    while (_isStarted)
    {
        int nready = poll(&fds, 1, 5000);
        if (-1 == nready && errno == EINTR)
        {
            continue;
        }
        else if (-1 == nready)
        {
            perror("-1 == nready");
            return;
        }
        else if (0 == nready)
        {
            continue;
        }
        else 
        {
            if (fds.revents == POLLIN)
            {
                _writeCacheTime += _interval;
                handleRead(_timefd);
                for (auto & task : _wheelList)
                {
                    std::cout << "update caches" <<  "\n";
                    task->process();
                    CacheManager::createCacheManger()->getMainCache().getCacheElem();            
                }
                /* for (auto & timerCallback: _timerCallbacks[fds[0].fd]) */
                /* { */
                /*     std::cout << "update caches" <<  "\n"; */
                /*     /1* task->process(); *1/ */
                /*     timerCallback(); */
                /*     CacheManager::createCacheManger()->getMainCache().getCacheElem(); */            
                /* } */
            }
            /* else if (fds[1].revents == POLLIN) */
            /* { */
            /*     handleRead(fds[0].fd); */
            /*     for (auto & writeCallback : _timerCallbacks[fds[1].fd]) */
            /*     { */
            /*         std::cout << "write Cache to dask" << "\n"; */
            /*         writeCallback(); */
            /*     } */
                                
            /* } */
        }
    }
}

void TimerManager::handleRead(int fd)
{
    uint64_t value;
    int ret = read(_timefd, &value, sizeof(value));
    if (ret != sizeof(value))
    {
        perror("read");
        return;
    }
}

void TimerManager::stop()
{
    _isStarted = false;
    setTimerfd(_timefd, 0, 0);
    return;
}

void TimerManager::registerTimer(int fd, TimerCallback timer)
{
    auto iter = _timerCallbacks.find(fd);
    if (iter == _timerCallbacks.end())
    {
        _timerCallbacks[fd].push_back(timer);
    }
    return;
}

void TimerManager::unregisterTimer(int fd, TimerCallback timer)
{
    auto iter = _timerCallbacks.find(fd);
    if (iter != _timerCallbacks.end())
    {
        _timerCallbacks.erase(iter);
    }
    return;
}

void TimerManager::attach(TimerTask * task)
{
    auto iter = std::find(_wheelList.begin(), _wheelList.end(), task);
    if (iter == _wheelList.end())
    {
        _wheelList.push_back(task);
    }
    return;
}

void TimerManager::detach(TimerTask * task)
{
    auto iter = std::find(_wheelList.begin(), _wheelList.end(), task);
    if (iter != _wheelList.end())
    {
        _wheelList.erase(iter);
    }
    return;
}

