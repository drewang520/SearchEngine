#ifndef _PAGELIB_PREPROCESSOR_H
#define _PAGELIB_PREPROCESSOR_H

#include "Configuration.h"
#include "CppJieBaSplit.h"
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>
#include <set>
#include <map>

using std::unordered_map;
using std::string;
using std::vector;
using std::pair;
using std::set;
using std::map;

class pageLibPreprocessor
{
public:
    pageLibPreprocessor(const Configuration& config);
    void buildInvertIndexMap();
    void store();

private:
    /* void dealContent(vector<string>& clearWords, int & docid_int, vector<map<string, int>>& deStopWords, */ 
    /*                     map<string, set<int>>& docFrequency); */
    void dealContent(map<string, int>& clearWords, int & docid_int, 
                        map<string, set<int>>& docFrequency);
    /* map<string, int>& CutClear(const string& content, const cppjieba::Jieba& jieba, */
    /*                              vector<string>& words, map<string, int>& clearWords, set<string>& stop_words); */

private:
    unordered_map<int, pair<int, int>> m_offsetLib;
    /* unordered_map<string, set< pair<int, double> >> m_invertIndexTable; */
    unordered_map<string, vector< pair<int, double> >> m_invertIndexTable;
    const Configuration& m_config;
};


#endif
