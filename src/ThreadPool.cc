#include "ThreadPool.h"
#include "Task.h"
#include "TaskQueue.h"
#include "Thread.h"
#include "WorkThread.h"
#include <memory>
#include <unistd.h>

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
        unique_ptr<Thread> ptr(new WorkThread(*this)); 
        _threads.push_back(std::move(ptr));
    }

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
    /* sleep(1); */
    /* while (_taskque.get_finish_threadNum() == _threadNums) */
    /* { */
    /*     _taskque.wakeup(); */
    /* } */
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

/* unique_ptr<Task> ThreadPool::getTask() */
/* { */
/*     return _taskque.pop(); */
/* } */

void ThreadPool::threadFunc()
{
    while (1)
    {
        unique_ptr<Task> ptr(std::move(_taskque.pop()));
        if (ptr)
        {
           ptr->process();
        }
        else
        {
            break;
        }
    }
}
