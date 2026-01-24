#ifndef __WORKTHREAD_H__
#define __WORKTHREAD_H__

#include "Thread.h"
#include "ThreadPool.h"
#include "TimerManager.h"
#include <iostream>

class WorkThread
: public Thread
{
public:
   WorkThread(ThreadPool & threadpool, const std::string& name)
   : _threadpool(threadpool)
   , Thread(name)
   {
       std::cout << "WorkThread()" << "\n";
   }
   void ChildWork() override
   {
        _threadpool.threadFunc();
   }
private:
   ThreadPool & _threadpool;
};

class TimerThread
: public Thread
{
public:
    TimerThread(const std::string& name)
    : Thread(name)
    {
        std::cout << "TimerThread()" << "\n";
    }

    void ChildWork() override
    {
        TimerManager::createTimerManager()->start();        
    }
};

#endif
