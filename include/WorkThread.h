#ifndef __WORKTHREAD_H__
#define __WORKTHREAD_H__

#include "Thread.h"
#include "ThreadPool.h"
#include <iostream>

class WorkThread
: public Thread
{
public:
   WorkThread(ThreadPool & threadpool)
   : _threadpool(threadpool)
   {
       std::cout << "WorkThread()" << std::endl;
   }
   void ChildWork() override
   {
        _threadpool.threadFunc();
   }
private:
   ThreadPool & _threadpool;
};

#endif
