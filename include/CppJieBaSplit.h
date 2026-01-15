#ifndef _CPPJIEBA_SPLIT_H
#define _CPPJIEBA_SPLIT_H

#include "Configuration.h"
#include "NoCopyable.h"
#include "cppjieba/Jieba.hpp"
#include <utility>
#include <vector>
#include <string>
#include <map>

using std::string;
using std::vector;
using std::pair;
using std::map;

class CppJiebaSplit
: public NoCopyable
{
public:
    CppJiebaSplit(const Configuration * config);

    void cut(const string& key, vector<string>& clearWords, const set<string>& stop_words) const;
    void cut(const string& key, map<string, int>& words, const set<string>& stop_words) const;
    

private:
    const Configuration * _config;
    cppjieba::Jieba  _jieba;
};

#endif
