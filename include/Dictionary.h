#ifndef _DICTIONARY_H
#define _DICTIONARY_H

#include "Configuration.h"
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <string>

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::set;

class Dictionary
{
public:
    Dictionary(const Configuration& config);
    vector<pair<string, int>> & getDict();
    map<string, set<int>> & getIndex();
    const vector<pair<string, int>> & getDict() const;
    const map<string, set<int>> & getIndex() const;

private:
    void init();
private:
    vector<pair<string, int>> m_dict;
    map<string, set<int>> m_index;
    const Configuration& m_config;

};

#endif
