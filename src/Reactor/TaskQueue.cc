#include "TaskQueue.h"
#include "Task.h"

TaskQueue::TaskQueue(size_t queSize)
: m_queSize(queSize)
, m_mutex()
, m_notEmpty(m_mutex)
, m_notFull(m_mutex)
, m_isRunning(true)
{

}

TaskQueue::~TaskQueue()
{
    
}

void TaskQueue::push(std::unique_ptr<Task> task)
{
    MutexLockGuard autolock(m_mutex);
    while(m_isRunning && isFullLocked())
    {
        m_notFull.wait();
    }

    if (!m_isRunning)
    {
        return;
    }

    m_taskQue.push(std::move(task));
    m_notEmpty.signal();
}

std::unique_ptr<Task> TaskQueue::pop()
{
    MutexLockGuard autolock(m_mutex);
    while(m_isRunning && isEmptyLocked())
    {
        m_notEmpty.wait(); 
    }

    if(isEmptyLocked())
    {
        return nullptr;
    }

    std::unique_ptr<Task> frontTask(std::move(m_taskQue.front()));
    m_taskQue.pop();
    m_notFull.signal();

    return frontTask;
}

bool TaskQueue::isEmpty()
{
    MutexLockGuard autolock(m_mutex);
    return isEmptyLocked();
}

bool TaskQueue::isFull()
{
    MutexLockGuard autolock(m_mutex);
    return isFullLocked();
}

void TaskQueue::wakeup()
{
    MutexLockGuard autolock(m_mutex);
    m_isRunning = false;
    m_notEmpty.signalAll();
    m_notFull.signalAll();
}

bool TaskQueue::isEmptyLocked() const
{
    return m_taskQue.empty();
}

bool TaskQueue::isFullLocked() const
{
    return m_taskQue.size() == m_queSize;
}
   
