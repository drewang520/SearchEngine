#ifndef __TASK_H__
#define __TASK_H__


class Task
{
public:
    virtual ~Task() {}
    virtual void process() = 0;
};


#endif

