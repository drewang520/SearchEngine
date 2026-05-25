#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <string>

extern thread_local const char * thread_name;

class Thread
{
public:
    Thread(const std::string& name);
    virtual ~Thread() {}
    void start();
    void end();
    virtual void ChildWork() = 0;
private:
    static void * threadFunc(void * argc);

private:
    pthread_t m_pid;
    bool m_isRunning;
    std::string m_name;
};




#endif
