#include "TaskQueue.h"
#include "Task.h"

TaskQueue::TaskQueue(size_t queSize)
: _queSize(queSize)
, _mutex()
, _notEmpty(_mutex)
, _notFull(_mutex)
, _flag(true)
{

}


TaskQueue::~TaskQueue()
{
    
}

void TaskQueue::push(unique_ptr<Task> task)
{
    /* _mutex.lock(); */
    //利用RAII的思想，资源分配即初始化
    AutoMutexLock autolock(_mutex);
    while(isFull())
    {
        _notFull.wait();
    }

    _taskque.push(std::move(task));

    _notEmpty.signal();

    /* _mutex.unlock(); */
}

unique_ptr<Task> TaskQueue::pop()
{
    /* _mutex.lock(); */
    AutoMutexLock autolock(_mutex);
    while(_flag && isEmpty())
    {
        _notEmpty.wait(); 
    }

    if(_flag)
    {
        unique_ptr<Task> front_task(std::move(_taskque.front()));

        _taskque.pop();

        _notFull.signal();
        /* _mutex.unlock(); */

        return front_task;
    }
    else
    {
        return nullptr;
    }
}

bool TaskQueue::isEmpty()
{
    return _taskque.empty();
}

bool TaskQueue::isFull()
{
    return _taskque.size() == _queSize;
}

void TaskQueue::wakeup()
{
    _flag = false;
    _notEmpty.signalAll();
}
   
