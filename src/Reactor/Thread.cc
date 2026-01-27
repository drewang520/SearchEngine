#include "Thread.h"
#include <cstdio>
#include <pthread.h>

using std::perror;

thread_local const char * thread_name = "cacheID";

Thread::Thread(const std::string& name)
: _pid(0)
, isrunning(false)
, _name(name)
{
    
}

void Thread::start()
{
    int ret = pthread_create(&_pid, nullptr, threadFunc, (void *)this);
    if(ret != 0 )
    {
        perror("子线程创建失败: ");                 
    }
    isrunning = true;
}

void Thread::end()
{
    pthread_join(_pid, nullptr);
    isrunning = false;
}

void * Thread::threadFunc(void *argc)
{
    Thread * ptr = static_cast<Thread *>(argc);
    thread_name = ptr->_name.c_str();
    if(ptr)
    {
        ptr->ChildWork(); //多态
    }
    pthread_exit((void *)0);
}

