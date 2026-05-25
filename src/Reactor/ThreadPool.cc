#include "ThreadPool.h"
#include "WorkThread.h"
#include <memory>
#include <string>

ThreadPool::ThreadPool(std::size_t threadNums, std::size_t queSize)
: m_threadNums(threadNums)
, m_queSize(queSize)
, m_threads()
, m_taskQue(queSize)
{

}


ThreadPool::~ThreadPool()
{

}

void ThreadPool::start()
{
    for (std::size_t ret = 0; ret != m_threadNums; ++ret)
    {
        std::unique_ptr<Thread> ptr(new WorkThread(*this, std::to_string(ret))); 
        m_threads.push_back(std::move(ptr));
    }
    std::unique_ptr<Thread> TimerPtr(new TimerThread(std::to_string(m_threadNums)));
    m_threads.push_back(std::move(TimerPtr));
    for(auto &thread : m_threads)
    {
        thread->start();
    }
}

void ThreadPool::stop()
{
    TimerManager::createTimerManager()->stop();
    m_taskQue.wakeup();
    for (auto & thread : m_threads)
    {
       thread->end(); 
    }
}

void ThreadPool::addTask(std::unique_ptr<Task> task)
{
    if (task)
    {
        m_taskQue.push(std::move(task));
    }
}

void ThreadPool::threadFunc()
{
    while (1)
    {
        std::unique_ptr<Task> ptr(std::move(m_taskQue.pop()));
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
