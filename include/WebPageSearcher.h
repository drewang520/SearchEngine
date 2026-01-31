#ifndef _WEB_PAGE_SEARCHER_H
#define _WEB_PAGE_SEARCHER_H

#include "Configuration.h"
#include "CppJieBaSplit.h"
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
    WebPageQuery(const Configuration * config);
    vector<WebPage> doQuery(const string& key, const CppJiebaSplit& jieba);

private:
    unordered_map<string, vector<pair<int, double>>> _invertIndexLib;
    unordered_map<int, pair<int, int>> _offsetLib;
    const Configuration * _config;
};

class WebPageSearch
{
public:
    WebPageSearch(const string& keyword, const Configuration * config);
    vector<WebPage> doQuery();

private:
    string _sought;
    const Configuration * _config;
    WebPageQuery _webPageSearch;
    CppJiebaSplit _jieba;
};

#endif
