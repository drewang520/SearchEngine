#ifndef __NOCOPYABLE_H__
#define __NOCOPYABLE_H__

class NoCopyable
{
public:
    NoCopyable() {  }
    ~NoCopyable() {  }

    NoCopyable(const NoCopyable& rhs) = delete;
    NoCopyable & operator=(const NoCopyable & rhs) = delete;
};

#endif
