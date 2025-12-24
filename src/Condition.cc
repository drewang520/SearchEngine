#include "Condition.h"

Condition::Condition(MutexLock & mutex)
: _mutex(mutex) 
/* , _cond(PTHREAD_COND_INITIALIZER) */
{
    pthread_cond_init(&_cond, nullptr);
}

Condition::~Condition()
{
    pthread_cond_destroy(&_cond);
}

void Condition::wait()
{
    pthread_cond_wait(&_cond, _mutex.getPtr());
}

void Condition::signal()
{
    pthread_cond_signal(&_cond);
}

void Condition::signalAll()
{
    pthread_cond_broadcast(&_cond);
}
