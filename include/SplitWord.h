#ifndef _SPLIT_WORD_H
#define _SPLIT_WORD_H

#include <vector>
#include <string>
using std::string;
using std::vector;

class SplitWord
{
public:
    SplitWord() {}
    virtual ~SplitWord() {}
    virtual vector<string> cut(const string& key) = 0;
};

#endif
