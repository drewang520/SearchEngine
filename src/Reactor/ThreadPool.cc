#include "ThreadPool.h"
#include "WorkThread.h"
#include <unistd.h>
#include <memory>
#include <string>

ThreadPool::ThreadPool(size_t threadNums, size_t queSize)
: _threadNums(threadNums)
, _queSize(queSize)
, _threads()
, _taskque(_queSize)
{

}


ThreadPool::~ThreadPool()
{

}

void ThreadPool::start()
{
    for (size_t ret = 0; ret != _threadNums; ++ret)
    {
        unique_ptr<Thread> ptr(new WorkThread(*this, std::to_string(ret))); 
        _threads.push_back(std::move(ptr));
    }
    unique_ptr<Thread> TimerPtr(new TimerThread(std::to_string(_threadNums)));
    _threads.push_back(std::move(TimerPtr));
    for(auto &_thread : _threads)
    {
        _thread->start();
    }
}

void ThreadPool::stop()
{
    while(!_taskque.isEmpty())
    {
        sleep(1);
    }
    _taskque.wakeup();
    for (auto &_thread : _threads)
    {
       _thread->end(); 
    }
}

void ThreadPool::addTask(unique_ptr<Task> task)
{
    if (task)
    {
        _taskque.push(std::move(task));
    }
}

void ThreadPool::threadFunc()
{
    while (1)
    {
        unique_ptr<Task> ptr(std::move(_taskque.pop()));
        if (ptr)
        {
           ptr->process(); //多态
        }
        else
        {
            break;
        }
    }
}

