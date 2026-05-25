#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__

#include "Condition.h"
#include "Task.h"
#include <cstddef>
#include <memory>
#include <queue>

class TaskQueue
{
public:
    TaskQueue(std::size_t queSize);
    ~TaskQueue();
    void push(std::unique_ptr<Task> task);
    std::unique_ptr<Task> pop();
    bool isEmpty();
    bool isFull();
    void wakeup();

private:
    bool isEmptyLocked() const;
    bool isFullLocked() const;

private:
    std::size_t m_queSize;
    std::queue<std::unique_ptr<Task>> m_taskQue;
    MutexLock m_mutex;
    Condition m_notEmpty;
    Condition m_notFull;
    bool m_isRunning;
};

#endif
