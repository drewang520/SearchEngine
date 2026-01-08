#ifndef _KEY_RECOMMANDER_H
#define _KEY_RECOMMANDER_H

#include "EventLoop.h"
#include "Dictionary.h"
#include <string>
#include <queue>
#include <vector>
#include <memory>

class TcpConnection;
class Configuration;

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
    KeyRecommander(string queryWords, Configuration * config);
    vector<string> doQuery();

private:
    int editDistance(const string&, const string&);
    vector<string> candidataSort(map<string, int>&, vector<string>&);
    void queryIndex(map<string, int>& ,map<string ,set<int>>&, 
                        vector<pair<string, int>>&, const string&);

private:
    string _queryWords;
    std::priority_queue<CandidateResult, vector<CandidateResult>, CompareHot> _prique;
    unique_ptr<Dictionary> _pDict;
};

#endif
