#include "Thread.h"
#include "Logger.h"
#include <cstring>
#include <pthread.h>
#include <string>

thread_local const char * thread_name = "cacheID";

Thread::Thread(const std::string& name)
: m_pid(0)
, m_isRunning(false)
, m_name(name)
{
    
}

void Thread::start()
{
    int ret = pthread_create(&m_pid, nullptr, threadFunc, (void *)this);
    if(ret != 0 )
    {
        LOG_ERROR(std::string("pthread_create failed name=") + m_name
                  + " reason=" + std::strerror(ret));
        return;
    }
    m_isRunning = true;
}

void Thread::end()
{
    pthread_join(m_pid, nullptr);
    m_isRunning = false;
}

void * Thread::threadFunc(void *argc)
{
    Thread * ptr = static_cast<Thread *>(argc);
    if(ptr)
    {
        thread_name = ptr->m_name.c_str();
        ptr->ChildWork(); //多态
    }
    pthread_exit((void *)0);
}
