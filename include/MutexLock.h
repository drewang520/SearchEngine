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
    pthread_mutex_t m_mutex;
};

class MutexLockGuard
{
public:
    MutexLockGuard(MutexLock & mutex)
    : m_mutex(mutex)
    {
        m_mutex.lock();    
    }

    ~MutexLockGuard()
    {
        m_mutex.unlock();
    }
    MutexLockGuard(const MutexLockGuard& rhs) = delete;
    MutexLockGuard& operator=(const MutexLockGuard& rhs) = delete;
private:
    MutexLock & m_mutex;
};

#endif
