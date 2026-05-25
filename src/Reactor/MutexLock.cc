#include "MutexLock.h"
#include "Logger.h"
#include <cstring>
#include <string>

MutexLock::MutexLock()
{
    int ret = pthread_mutex_init(&m_mutex, nullptr);
    if(0 != ret)
    {
        LOG_ERROR(std::string("pthread_mutex_init failed: ") + std::strerror(ret));
    }
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&m_mutex);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

void MutexLock::trylock()
{
    int ret = pthread_mutex_trylock(&m_mutex);
    if(0 != ret)
    {
        LOG_ERROR(std::string("pthread_mutex_trylock failed: ") + std::strerror(ret));
    }
}

pthread_mutex_t * MutexLock::getPtr()
{
    return &m_mutex;
}
