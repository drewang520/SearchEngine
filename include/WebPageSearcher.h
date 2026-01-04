#ifndef _WEB_PAGE_SEARCHER_H
#define _WEB_PAGE_SEARCHER_H

#include "EventLoop.h"
#include "PageProducer.h"
#include <vector>
#include <unordered_map>
#include <utility>
#include <string>

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

class WebPageQuery
{
public:
    WebPageQuery(const string& invertIndex, const string& offsetLib);
    void doQuery(const string& key);

private:
    unordered_map<string, vector<pair<int, double>>> _invertIndexLib;
    unordered_map<int, pair<int, int>> _offsetLib;
};

class WebPageSearch
{
public:
    WebPageSearch();
    string doQuery();

private:
    string _sought;
    TcpConnectionPtr _conn;
};

#endif
