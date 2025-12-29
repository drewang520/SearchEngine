#ifndef _PAGELIB_PREPROCESSOR_H
#define _PAGELIB_PREPROCESSOR_H

#include "cppjieba/Jieba.hpp"
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>
#include <set>

using std::unordered_map;
using std::string;
using std::vector;
using std::pair;
using std::set;

class pageLibPreprocessor
{
public:
    pageLibPreprocessor();
    void buildInvertIndexMap(const string& newripepage, const string& newoffset);
    void store();
    void dealContent(vector<string>& words, const cppjieba::Jieba& jieba, 
                    int & docid_int, const string& content, set<string>& stop_words);

private:
    unordered_map<int, pair<int, int>> _offsetLib;
    unordered_map<string, set< pair<int, double> >> _invertIndexTable;
};


#endif
