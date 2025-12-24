#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "TaskQueue.h"
#include "Thread.h"
#include "Task.h"
#include <cstddef>
#include <memory>
#include <vector>

using std::vector;
using std::unique_ptr;

class ThreadPool
{
    friend class WorkThread;
public:
    ThreadPool(size_t _threadNums, size_t _queSize);
    ~ThreadPool();
    void start();
    void stop();
    void addTask(unique_ptr<Task> task); 
    unique_ptr<Task> getTask();

private:
    void threadFunc();

private:
    size_t _threadNums;
    vector<std::unique_ptr<Thread>> _threads;
    size_t _queSize;
    TaskQueue _taskque;
};



#endif
