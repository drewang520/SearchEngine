#ifndef __WORKTHREAD_H__
#define __WORKTHREAD_H__

#include "Logger.h"
#include "Thread.h"
#include "ThreadPool.h"
#include "TimerManager.h"

class WorkThread
: public Thread
{
public:
   WorkThread(ThreadPool & threadpool, const std::string& name)
   : m_threadpool(threadpool)
   , Thread(name)
   {
       LOG_DEBUG("work thread constructed name=" + name);
   }
   void ChildWork() override
   {
        m_threadpool.threadFunc();
   }
private:
   ThreadPool & m_threadpool;
};

class TimerThread
: public Thread
{
public:
    TimerThread(const std::string& name)
    : Thread(name)
    {
        LOG_DEBUG("timer thread constructed name=" + name);
    }

    void ChildWork() override
    {
        TimerManager::createTimerManager()->start();        
    }
};

#endif
