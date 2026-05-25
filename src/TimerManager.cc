#include "TimerManager.h"
#include "CacheManager.h"
#include "Configuration.h"
#include "LRUCache.h"
#include "Logger.h"
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

TimerManager * TimerManager::m_timerManager = nullptr;
pthread_once_t TimerManager::once_init = PTHREAD_ONCE_INIT;

TimerManager * TimerManager::createTimerManager()
{
    pthread_once(&once_init, init);
    return m_timerManager;
}

void TimerManager::destory()
{
    if (m_timerManager)
    {
        delete m_timerManager;
        m_timerManager = nullptr;
    }
}

void TimerManager::init()
{
    m_timerManager = new TimerManager();
    atexit(destory);
}

TimerManager::TimerManager()
: m_timefd(createTimerfd())
, m_writefd(createTimerfd())
, m_isStarted(false)
, m_writeCacheTime(0)
, m_delay(std::stoi(Configuration::createpInstance().getConfig().at("delay")))
, m_interval(std::stoi(Configuration::createpInstance().getConfig().at("interval")))
, m_writeCachedelay(std::stoi(Configuration::createpInstance().getConfig().at("writeCachedelay")))
, m_writeCacheinterval(std::stoi(Configuration::createpInstance().getConfig().at("writeCacheinterval")))
{
    m_timerCallbacks[m_timefd] = std::vector<TimerCallback>();
    m_timerCallbacks[m_writefd] = std::vector<TimerCallback>();
}

int TimerManager::createTimerfd()
{
    int timerfd = timerfd_create(CLOCK_REALTIME, 0);    
    if (-1 == timerfd)
    {
        LOG_ERROR("timerfd_create failed");
    }
    return timerfd;
}

int TimerManager::getWriteCacheTime()
{
    return m_writeCacheTime;
}

void TimerManager::clearCacheTime()
{
    m_writeCacheTime = 0;
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
        LOG_ERROR("timerfd_settime failed fd=" + std::to_string(fd));
    }
}

void TimerManager::start()
{
    LOG_INFO("timer manager start delay=" + std::to_string(m_delay)
             + " interval=" + std::to_string(m_interval));
    struct pollfd fds;       
    fds.fd = m_timefd;
    fds.events = POLLIN;
    /* fds[0].fd = m_timefd; */
    /* fds[0].events = POLLIN; */
    
    /* fds[1].fd = m_writefd; */
    /* fds[1].events = POLLIN; */
    CacheUpdateTask cacheUpdatetask;
    /* WriteCacheTask writeCacheTask; */
    m_timerManager->attach(&cacheUpdatetask);
    /* m_timerManager->attach(&writeCacheTask); */
    /* registerTimer(m_timefd, std::bind(&CacheManager::PeriodicalUpdateCache, CacheManager::createCacheManger())); */
    /* registerTimer(m_writefd, std::bind(&LRUCache::writeToFile, CacheManager::getMainCache(), */ 
    /*                                              Configuration::createpInstance().getConfig().at("cacheData"))); */
    setTimerfd(m_timefd, m_delay, m_interval);
    /* setTimerfd(fds[1].fd, m_writeCachedelay, m_writeCacheinterval); */
    
    m_isStarted = true;
    while (m_isStarted)
    {
        int nready = poll(&fds, 1, 5000);
        if (-1 == nready && errno == EINTR)
        {
            continue;
        }
        else if (-1 == nready)
        {
            LOG_ERROR("timer poll failed");
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
                m_writeCacheTime += m_interval;
                handleRead(m_timefd);
                for (auto & task : m_wheelList)
                {
                    LOG_INFO("timer triggered cache update elapsed="
                             + std::to_string(m_writeCacheTime));
                    task->process();
                    LOG_DEBUG("main cache size="
                              + std::to_string(CacheManager::getMainCache().getResultList().size()));            
                }
                /* for (auto & timerCallback: m_timerCallbacks[fds[0].fd]) */
                /* { */
                /*     std::cout << "update caches" <<  "\n"; */
                /*     /1* task->process(); *1/ */
                /*     timerCallback(); */
                /*     CacheManager::getMainCache().getCacheElem(); */            
                /* } */
            }
            /* else if (fds[1].revents == POLLIN) */
            /* { */
            /*     handleRead(fds[0].fd); */
            /*     for (auto & writeCallback : m_timerCallbacks[fds[1].fd]) */
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
    int ret = read(m_timefd, &value, sizeof(value));
    if (ret != sizeof(value))
    {
        LOG_ERROR("timerfd read failed");
        return;
    }
}

void TimerManager::stop()
{
    LOG_INFO("timer manager stop");
    m_isStarted = false;
    setTimerfd(m_timefd, 0, 0);
    return;
}

void TimerManager::registerTimer(int fd, TimerCallback timer)
{
    auto iter = m_timerCallbacks.find(fd);
    if (iter == m_timerCallbacks.end())
    {
        m_timerCallbacks[fd].push_back(timer);
    }
    return;
}

void TimerManager::unregisterTimer(int fd, TimerCallback timer)
{
    auto iter = m_timerCallbacks.find(fd);
    if (iter != m_timerCallbacks.end())
    {
        m_timerCallbacks.erase(iter);
    }
    return;
}

void TimerManager::attach(TimerTask * task)
{
    auto iter = std::find(m_wheelList.begin(), m_wheelList.end(), task);
    if (iter == m_wheelList.end())
    {
        m_wheelList.push_back(task);
    }
    return;
}

void TimerManager::detach(TimerTask * task)
{
    auto iter = std::find(m_wheelList.begin(), m_wheelList.end(), task);
    if (iter != m_wheelList.end())
    {
        m_wheelList.erase(iter);
    }
    return;
}
