#ifndef _DIC_PRODUCER_H
#define _DIC_PRODUCER_H

#include "Configuration.h"
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <string>

using std::pair;
using std::string;
using std::set;
using std::vector;
using std::map;

class DicProducer
{
public:
    DicProducer(const string&, const Configuration& config);
    void buildEnDict(const set<string>& stopWords);
    void buildCnDict(const set<string>& stopWords);
    void createEnIndex();
    void createCnIndex();
    void storeDict(string savefile);
    void storeIndex(string savefile);
    void printFile() const;
    void buildDictAndIndex();
    void storeAllDict();
    void storeALlIndex();
    /* void CnDispatch(string sentence); */
private:
    string dealWord(string word);

private:
    vector<string> m_files;
    map<string, int> m_dict;
    vector<pair<string, int>> m_dict2;
    map<string, set<int>> m_index;
    const Configuration& m_config;
    // SplitTool * _cuttor;
};

#endif
