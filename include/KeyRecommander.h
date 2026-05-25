#ifndef _KEY_RECOMMANDER_H
#define _KEY_RECOMMANDER_H

#include "Dictionary.h"
#include <string>
#include <queue>
#include <vector>

using std::vector;
using std::string;
using std::priority_queue;

using CandidateResult = struct CandidateResult
{
    string m_word;
    int m_freq;
    int m_dist;
};

struct CompareHot
{
    bool operator()(const CandidateResult& lhs, const CandidateResult& rhs) const
    {
        if (lhs.m_dist != rhs.m_dist)
        {
            return lhs.m_dist > rhs.m_dist;
        }
        if (lhs.m_freq != rhs.m_freq)
        {
            return lhs.m_freq < rhs.m_freq;
        }
        return lhs.m_word > rhs.m_word;
    }
};

class KeyRecommander
{
public:
    KeyRecommander(string queryWords, const Configuration& config);
    string startQuery(int cacheID);
    vector<string> doQuery();

private:
    int editDistance(const string&, const string&) const;
    void candidataSort(const map<string, int>&, vector<string>&) const;
    void queryIndex(map<string, int>& , const map<string ,set<int>>&, 
                        const vector<pair<string, int>>&, const string&) const;

private:
    string m_queryWords;
    const Dictionary& m_dict;
    const Configuration& m_config;
};

#endif
