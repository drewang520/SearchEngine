#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "TaskQueue.h"
#include "Thread.h"
#include "Task.h"
#include <cstddef>
#include <memory>
#include <vector>

class ThreadPool
{
    friend class WorkThread;
public:
    ThreadPool(std::size_t threadNums, std::size_t queSize);
    ~ThreadPool();
    void start();
    void stop();
    void addTask(std::unique_ptr<Task> task); 
    std::unique_ptr<Task> getTask();

private:
    void threadFunc();

private:
    std::size_t m_threadNums;
    std::vector<std::unique_ptr<Thread>> m_threads;
    std::size_t m_queSize;
    TaskQueue m_taskQue;
};



#endif
