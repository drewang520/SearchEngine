#ifndef _TIMER_MANAGER_H
#define _TIMER_MANAGER_H

#include "CacheManager.h"
#include "Configuration.h"
#include "Task.h"
#include <functional>
#include <pthread.h>
#include <vector>
#include <map>

using std::vector;

class TimerTask
: public Task
{
public:
    virtual ~TimerTask() {}
    virtual void process() = 0;
};

class CacheUpdateTask
: public TimerTask
{
public:
    virtual ~CacheUpdateTask() {}
    void process() override
    {
        CacheManager::createCacheManger().PeriodicalUpdateCache();        
    }
};

class WriteCacheTask
: public TimerTask
{
public:
    virtual ~WriteCacheTask() {}
    void process() override
    {
        CacheManager::getMainCache().writeToFile(
                 Configuration::createpInstance().getConfig().at("cacheData"));
    }
};

using TimerCallback = std::function<void ()>;

class TimerManager
{
     
public:
    static TimerManager * createTimerManager();
    static void destory();
    static void init();
    int getWriteCacheTime();
    void clearCacheTime();

    void start();
    void stop();
    void attach(TimerTask *);
    void detach(TimerTask *);

    
    void registerTimer(int fd, TimerCallback timer);
    void unregisterTimer(int fd, TimerCallback timer);
private:
    void handleRead(int);
    int createTimerfd();
    void setTimerfd(int fd, int delay, int interval);
    TimerManager();
    TimerManager(const TimerManager& timerManager) = delete;
    TimerManager& operator=(const TimerManager& timerManager) = delete;
    ~TimerManager() = default;

private:
    int m_timefd;
    int m_interval;//间隔时间
    int m_writeCacheTime;
    int m_delay; //首次到期时间
    int m_writefd;
    int m_writeCachedelay;
    int m_writeCacheinterval;
    bool m_isStarted;
    std::map<int, vector<TimerCallback>> m_timerCallbacks;
    vector<TimerTask *> m_wheelList;
    static TimerManager * m_timerManager;
    static pthread_once_t once_init;
};

#endif
