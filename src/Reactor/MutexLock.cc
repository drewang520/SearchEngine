#include "MutexLock.h"
#include <cstdio>

MutexLock::MutexLock()
{
    int ret = pthread_mutex_init(&_mutex, nullptr);
    if(0 != ret)
    {
        perror("创建锁失败： ");
    }
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&_mutex);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&_mutex);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&_mutex);
}

void MutexLock::trylock()
{
    int ret = pthread_mutex_trylock(&_mutex);
    if(0 != ret)
    {
        perror("尝试加锁失败： ");
    }
}

pthread_mutex_t * MutexLock::getPtr()
{
    return &_mutex;
}
