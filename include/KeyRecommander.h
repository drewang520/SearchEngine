#include "EventLoop.h"
#include <string>
#include <queue>

class TcpConnection;
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
    KeyRecommander(string, TcpConnectionPtr);
    string doQuery();

private:
    string _sought;
    std::priority_queue<CandidateResult> _prique;
    TcpConnectionPtr _conn;
};
