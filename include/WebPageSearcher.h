#ifndef _WEB_PAGE_SEARCHER_H
#define _WEB_PAGE_SEARCHER_H
/* #include "EventLoop.h" */
#include "Configuration.h"
#include "PageProducer.h"
#include "WebPage.h"
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
    WebPageQuery(Configuration * config);
    vector<WebPage> doQuery(const string& key);

private:
    unordered_map<string, vector<pair<int, double>>> _invertIndexLib;
    unordered_map<int, pair<int, int>> _offsetLib;
    Configuration * _config;
};

class WebPageSearch
{
public:
    WebPageSearch(const string& keyword, Configuration * config);
    vector<WebPage> doQuery();

private:
    string _sought;
    /* TcpConnectionPtr _conn; */
    Configuration * _config;
};

#endif
