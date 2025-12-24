#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__

#include "Condition.h"
#include "Task.h"
#include <memory>
#include <pthread.h>
#include <queue>

using std::unique_ptr;
using std::queue;

class TaskQueue
{
public:
    TaskQueue(size_t queSize);
    ~TaskQueue();
    void push(std::unique_ptr<Task> task);
    unique_ptr<Task> pop();
    bool isEmpty();
    bool isFull();
    void wakeup();

    size_t get_finish_threadNum();

private:
    size_t _queSize;
    queue<unique_ptr<Task>> _taskque;
    MutexLock _mutex;
    Condition _notEmpty;
    Condition _notFull;
    bool _flag;
    size_t _finishNum;
};

class AutoMutexLock
{
public:
    AutoMutexLock(MutexLock & mutex)
    : _mutex(mutex)
    {
        _mutex.lock();
    }
    ~AutoMutexLock()
    {
        _mutex.unlock();
    }
    AutoMutexLock(const AutoMutexLock& rhs) = delete;
    AutoMutexLock& operator=(const AutoMutexLock& rhs) = delete;

private:
    MutexLock & _mutex;
};
#endif
