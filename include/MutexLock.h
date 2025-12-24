#ifndef __MUTEXLOCK_H__
#define __MUTEXLOCK_H__

#include "NoCopyable.h"
#include <pthread.h>

class MutexLock
: public NoCopyable
{
public:
    MutexLock();
    ~MutexLock();

    void lock();
    void unlock();
    void trylock();
    
    pthread_mutex_t * getPtr();
private:
    pthread_mutex_t _mutex;
};

class MutexLockGuard
: public MutexLock
{
public:
    MutexLockGuard(MutexLock &mutex)
    : _mutex(mutex)
    {
        _mutex.lock();    
    }

    ~MutexLockGuard()
    {
        _mutex.unlock();
    }
private:
    MutexLock &_mutex;
};

#endif
