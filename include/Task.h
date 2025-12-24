#ifndef __TASK_H__
#define __TASK_H__


class Task
{
public:
    Task() {}
    virtual ~Task() {}
    virtual void process() = 0;
};


#endif

