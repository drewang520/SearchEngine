#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

class Thread
{
public:
    Thread();
    virtual ~Thread();
    void start();
    void end();
    virtual void ChildWork() = 0;
private:
    static void * threadFunc(void * argc);

private:
    pthread_t _pid;
    bool isrunning;
};




#endif
