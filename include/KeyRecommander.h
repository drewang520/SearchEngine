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

struct CandidateResult
{
    string _word;
    int _freq;
    int _dist;
};

class KeyRecommander
{
public:
    KeyRecommander(string queryWords, const TcpConnectionPtr& conn, Configuration * config);
    vector<string> doQuery();

private:
    string _queryWords;
    std::priority_queue<CandidateResult> _prique;
    TcpConnectionPtr _conn;
    Configuration * _config;
    unique_ptr<Dictionary> _pDict;
};
