#ifndef _KEY_RECOMMANDER_H
#define _KEY_RECOMMANDER_H

#include "Dictionary.h"
#include <string>
#include <queue>
#include <vector>
#include <memory>


using std::vector;
using std::unique_ptr;
using std::string;
using std::priority_queue;

using CandidateResult = struct CandidateResult
{
    string _word;
    int _freq;
    int _dist;
};

struct CompareHot
{
    bool operator()(const CandidateResult& lhs, const CandidateResult& rhs) const
    {
        if (lhs._dist != rhs._dist)
        {
            return lhs._dist > rhs._dist;
        }
        return lhs._freq < rhs._freq;               
    }
};

class KeyRecommander
{
public:
    KeyRecommander(string queryWords, const Configuration * config);
    vector<string> doQuery();

private:
    int editDistance(const string&, const string&);
    void candidataSort(map<string, int>&, vector<string>&);
    void queryIndex(map<string, int>& ,map<string ,set<int>>&, 
                        vector<pair<string, int>>&, const string&);

private:
    string _queryWords;
    priority_queue<CandidateResult, vector<CandidateResult>, CompareHot> _prique;
    unique_ptr<Dictionary> _pDict;
    const Configuration * _config;
};

#endif
