#include "CppJieBaSplit.h"
#include "Logger.h"
#include "WebPageSearcher.h"
#include <math.h>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <unordered_map>

using std::ifstream;
using std::istringstream;

namespace
{
WebPageQuery& getSharedWebPageQuery(const Configuration& config)
{
    static WebPageQuery webPageQuery(config);
    return webPageQuery;
}

CppJiebaSplit& getThreadLocalJieba(const Configuration& config)
{
    thread_local CppJiebaSplit jieba(config);
    return jieba;
}
}

WebPageSearch::WebPageSearch(const string& keyword, const Configuration& config)
: m_sought(keyword)
, m_config(config)
, m_webPageSearch(getSharedWebPageQuery(m_config))
, m_jieba(getThreadLocalJieba(m_config))
{
  
}

vector<WebPage> WebPageSearch::doQuery()
{
    return m_webPageSearch.doQuery(m_sought, m_jieba);
}

WebPageQuery::WebPageQuery(const Configuration& config)
: m_config(config)
{
    ifstream ifs(m_config.getConfig().at("invertIndexTable"));
    string line, word;
    int docid;
    double weight;
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        vector<pair<int, double>> index;
        iss >> word;
        while (iss >> docid >> weight)
        {
            index.push_back({docid, weight});
        }
        m_invertIndexLib[word] = index;
    }
    ifs.close();

    ifs.open(m_config.getConfig().at("newoffset"));
    int pos, offset;
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        iss >> docid >> pos >> offset;
        m_offsetLib[docid] = {pos, offset};
    }
    ifs.close();
}

vector<WebPage> WebPageQuery::doQuery(const string& key, const CppJiebaSplit& jieba)
{
    vector<string> clearWords;
    vector<WebPage> webPage;

    jieba.cut(key, clearWords);
    if (clearWords.empty())
    {
        LOG_WARN("web search no valid query words raw=" + key);
        return {};
    }

    vector<string> queryWords;
    std::unordered_map<string, int> queryFreq;
    queryWords.reserve(clearWords.size());
    for (const auto& word : clearWords)
    {
        auto [iter, inserted] = queryFreq.insert({word, 1});
        if (inserted)
        {
            queryWords.push_back(word);
        }
        else
        {
            ++iter->second;
        }
    }

    map<int, vector<double>> doc_weight;
    vector<string> indexedWords;
    vector<const vector<pair<int, double>>*> postingsList;
    indexedWords.reserve(queryWords.size());
    postingsList.reserve(queryWords.size());

    LOG_DEBUG("web search raw=" + key
              + " uniqueWords=" + std::to_string(queryWords.size()));

    for (const auto& word : queryWords)
    {
        auto it = m_invertIndexLib.find(word);
        if (it == m_invertIndexLib.end())
        {
            continue;
        }
        indexedWords.push_back(word);
        postingsList.push_back(&it->second);
    }

    if (indexedWords.empty())
    {
        LOG_WARN("web search no indexed query words raw=" + key);
        return {};
    }

    size_t queryLen = indexedWords.size();
    for (size_t idx = 0; idx < postingsList.size(); ++idx)
    {
        const auto& postings = *postingsList[idx];
        for (auto [docid, weight] : postings)
        {
            if (doc_weight.find(docid) == doc_weight.end())
            {
                doc_weight[docid] = vector<double>(queryLen, 0.0);
            }
            doc_weight[docid][idx] = weight;
        }
    }

    //计算查询词的权重向量
    double pageNum = static_cast<double>(m_offsetLib.size());
    if (pageNum <= 0.0)
    {
        return {};
    }

    vector<double> weight_query(indexedWords.size(), 0.0);
    /* std::cout << "dic_query is ok" << "\n"; */
    for (size_t idx = 0; idx < indexedWords.size(); ++idx)
    {
        double docFre = static_cast<double>(postingsList[idx]->size());
        double termFre = static_cast<double>(queryFreq[indexedWords[idx]]);
        double idf = log2((pageNum + 1.0) / (docFre + 1.0)) + 1.0;
        weight_query[idx] = termFre * idf;
        /* std::cout << weight_query[idx] << "\n"; */
    }
    /* std::cout << "weight_query is ok" << "weight_query.size = " << weight_query.size()  << "\n"; */

    /* std::cout << "doc_weight.size = " << doc_weight.size() << "\n"; */
    // 计算余弦相似度
    vector<pair<int, double>> cosins;
    for (const auto & [docid, weights] : doc_weight)
    {
        double cos = 0.0;
        double queryNorm2 = 0.0;
        double docNorm2 = 0.0;
        for (size_t i = 0; i < weight_query.size(); ++i)
        {
            cos += (weights[i] * weight_query[i]);
            queryNorm2 += (weight_query[i] * weight_query[i]);
            docNorm2 += (weights[i] * weights[i]);
        }
        double denominator = sqrt(docNorm2) * sqrt(queryNorm2);
        if (denominator > 0)
        {
            cosins.push_back({docid, (cos / denominator)});
        }
    }

    /* std::cout << "cosins is ok" << "cosins.size = " << cosins.size() << "\n"; */
    std::sort(cosins.begin(), cosins.end(), 
                [](const pair<int, double>& a, const pair<int, double>& b){
                     if (a.second == b.second)
                     {
                         return a.first < b.first;
                     }
                     return a.second > b.second;
              });

    // 选取余弦值最大的前10个文章并json化传个客户端
    // 这里是要访问磁盘的地方，缓存在这里体现出重要性
    ifstream ifs(m_config.getConfig().at("newripepage"));    
    if (!ifs)
    {
        return webPage;
    }

    size_t resultLimit = static_cast<size_t>(std::stoi(m_config.getConfig().at("queryWebPageNum")));
    resultLimit = std::min(resultLimit, cosins.size());
    LOG_INFO("web search raw=" + key
             + " indexedWords=" + std::to_string(indexedWords.size())
             + " matchedDocs=" + std::to_string(cosins.size())
             + " resultLimit=" + std::to_string(resultLimit));
    for (size_t i = 0; i < resultLimit; ++i)
    {
        auto offsetIt = m_offsetLib.find(cosins[i].first);
        if (offsetIt == m_offsetLib.end())
        {
            continue;
        }
        if (offsetIt->second.second <= 0)
        {
            continue;
        }
        ifs.seekg(offsetIt->second.first);
        string doc(offsetIt->second.second, '\0');
        if (!ifs.read(&doc[0], offsetIt->second.second))
        {
            continue;
        }
        WebPage web(doc);
        if (!web.getTitle().empty())
        {
            webPage.push_back(std::move(web));        
        }
    }
    return webPage;
}
