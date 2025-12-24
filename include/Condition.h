#ifndef __CONDITION_H__
#define __CONDITION_H__

#include "MutexLock.h"
#include "NoCopyable.h"

class Condition
: public NoCopyable
{
public:
    Condition(MutexLock & mutex);
    ~Condition();
    
    void wait();
    void signal();
    void signalAll();
    
private:
    pthread_cond_t _cond;
    MutexLock & _mutex;
};

#endif
