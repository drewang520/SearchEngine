#include "CppJieBaSplit.h"
#include "WebPageSearcher.h"
#include <math.h>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>

using std::ifstream;
using std::istringstream;
using std::set;

WebPageSearch::WebPageSearch(const string& keyword, const Configuration * config)
: _sought(keyword)
, _config(config)
, _jieba(_config)
{
  
}

vector<WebPage> WebPageSearch::doQuery()
{
    WebPageQuery webPageQuery(_config);
    return webPageQuery.doQuery(_sought, _jieba);
}

WebPageQuery::WebPageQuery(const Configuration * config)
: _config(config)
{
    ifstream ifs(_config->getConfig().at("invertIndexTable"));
    string line;
    string word, docid, weight;
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        iss >> word;
        while (iss >> docid >> weight)
        {
            _invertIndexLib[word].emplace_back(stoi(docid), stod(weight));
        }
    }
    ifs.close();

    ifs.open(_config->getConfig().at("newoffset"));
    string pos, offset;
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        while (iss >> docid >> pos >> offset)
        {
            _offsetLib[stoi(docid)].first = stoi(pos);
            _offsetLib[stoi(docid)].second= stoi(offset);
        }
    }
    ifs.close();
}

vector<WebPage> WebPageQuery::doQuery(const string& key, const CppJiebaSplit& jieba)
{
    set<string> stopWords = _config->getStopWords();
    vector<string> clearWords;
    vector<WebPage> webPage;

    jieba.cut(key, clearWords, stopWords);
    if (clearWords.empty())
    {
        std::cout << "the word is stopWords" << "\n";
        return {};
    }

    map<int, vector<double>> doc_weight;
    size_t queryLen = clearWords.size();

    for (auto elem : clearWords)
    {
        std::cout << elem << " ";
    }
    std::cout << "\n";
    /* std::cout << "\n" << "queryLen = " << queryLen << "\n"; */
    for (size_t idx = 0; idx < clearWords.size(); ++idx)
    {
        const string& word = clearWords[idx];
        auto it = _invertIndexLib.find(word);
        if (it == _invertIndexLib.end())
        {
            std::cout << "key is exzit" << "\n";
            return {}; 
            /* continue; */
        }
        for (auto [docid, weight] : it->second)
        {
            if (doc_weight.find(docid) == doc_weight.end())
            {
                doc_weight[docid] = vector<double>(queryLen, 0.0);
            }
            doc_weight[docid][idx] = weight;
        }
    }

    //计算查询词的权重向量
    size_t pageNum = _offsetLib.size();
    map<string, int> dic_query;
    vector<double> weight_query(clearWords.size(), 0.0);
    for (auto elem : clearWords)
    {
        pair<map<string, int>::iterator, bool> p = dic_query.insert({elem, 1});
        if (! p.second)
        {
            ++p.first->second;
        }
    }
    /* std::cout << "dic_query is ok" << "\n"; */
    for (size_t idx = 0; idx < clearWords.size(); ++idx)
    {
        size_t docFre = _invertIndexLib[clearWords[idx]].size();
        size_t termFre = dic_query[clearWords[idx]];
        double idf = log2(pageNum / (docFre + 1));
        weight_query[idx] = dic_query[clearWords[idx]] * idf;
        /* std::cout << weight_query[idx] << "\n"; */
    }
    /* std::cout << "weight_query is ok" << "weight_query.size = " << weight_query.size()  << "\n"; */

    vector<double> doc_weight_mo;
    for (const auto & [docid, weight] : doc_weight)
    {
        double powNum = 0.0;
        for (const auto &elem : weight)
        {
             powNum += (elem * elem);
        }
        doc_weight_mo.push_back(sqrt(powNum));
    }

    /* std::cout << "doc_weight.size = " << doc_weight.size() << "\n"; */
    // 计算余弦相似度
    vector<pair<int, double>> cosins;
    size_t idx = 0;
    for (const auto & [docid, weights] : doc_weight)
    {
        double cos = 0.0;
        double weight_mo = 0.0;
        for (size_t i = 0; i < weight_query.size(); ++i)
        {
            cos += (weights[i] * weight_query[i]);
            weight_mo += (weight_query[i] * weight_query[i]);
        }
        cosins.push_back({docid, (cos / (doc_weight_mo[idx] * sqrt(weight_mo)))});
        ++idx;
    }

    /* std::cout << "cosins is ok" << "cosins.size = " << cosins.size() << "\n"; */
    std::sort(cosins.begin(), cosins.end(), 
                [](const pair<int, double>& a, const pair<int, double>& b){
                     return a.second > b.second;
              });

    // 选取余弦值最大的前10个文章并json化传个客户端
    // 这里是要访问磁盘的地方，缓存在这里体现出重要性
    ifstream ifs(_config->getConfig().at("newripepage"));    
    for (size_t i = 0; i < stoi(_config->getConfig().at("queryWebPageNum")); ++i)
    {
        ifs.seekg(_offsetLib[cosins[i].first].first - 1);
        char * buf = new char[_offsetLib[cosins[i].first].second + 1]();
        ifs.read(buf, _offsetLib[cosins[i].first].second);
        WebPage web(buf);
        if (!web.getTitle().empty())
        {
            webPage.push_back(std::move(web));        
        }
        delete [] buf;
    }
    return webPage;
}


