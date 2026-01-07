#include "WebPageSearcher.h"
#include "cppjieba/Jieba.hpp"
#include <math.h>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <algorithm>

using std::ifstream;
using std::istringstream;
using std::set;

WebPageQuery::WebPageQuery(const string& invertIndex, const string& offsetLib)
{
    ifstream ifs1(invertIndex);
    ifstream ifs2(offsetLib);
    string line;
    string word, docid, weight;
    while (std::getline(ifs1, line))
    {
        istringstream iss(line);
        iss >> word;
        while (iss >> docid >> weight)
        {
            _invertIndexLib[word].emplace_back(stoi(docid), stod(weight));
        }
    }
    string pos, offset;
    while (std::getline(ifs2, line))
    {
        istringstream iss(line);
        while (iss >> docid >> pos >> offset)
        {
            _offsetLib[stoi(docid)].first = stoi(pos);
            _offsetLib[stoi(docid)].second= stoi(offset);
        }
    }
    //test
    /* for (const auto & [word, position]: _invertIndexLib) */
    /* { */
    /*     std::cout << word << " "; */
    /*     for (const auto & [docid, weight] : position) */
    /*     { */
    /*         std::cout << docid << " " << weight << " "; */
    /*     } */
    /*     std::cout << "\n"; */
    /* } */
}

void WebPageQuery::doQuery(const string& key)
{
             
    const char * dict_path = "../raw_data/module1/dict/jieba.dict.utf8";
    const char * model_path = "../raw_data/module1/dict/hmm_model.utf8";
    const char * user_dict_path = "../raw_data/module1/dict/user.dict.utf8";
    const char * idf_path = "../raw_data/module1/dict/idf.utf8";
    const char * stop_word_path = "../raw_data/module1/dict/stop_words.utf8";
    cppjieba::Jieba jieba(dict_path, model_path, user_dict_path, idf_path, stop_word_path);

    vector<string> words;
    set<string> _stopWords;
    ifstream ifs3(stop_word_path);
    string line;
    while (std::getline(ifs3, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        _stopWords.insert(line);
    }
    
    vector<string> clearWords;
    jieba.Cut(key, words, true);
    for (size_t index = 0; index < words.size(); ++index)
    {
        if (!words[index].empty() && words[index].back() == '\r' || words[index].back() == '\n')
        {
            words[index].pop_back();
        }
        if (_stopWords.find(words[index]) == _stopWords.end())
        {
            clearWords.push_back(words[index]);
        }
    }

    map<int, vector<double>> doc_weight;
    size_t queryLen = clearWords.size();

    for (auto elem : clearWords)
    {
        std::cout << elem << " ";
    }
    std::cout << "\n" << "queryLen = " << queryLen << "\n";
    for (size_t idx = 0; idx < clearWords.size(); ++idx)
    {
        const string& word = clearWords[idx];
        auto it = _invertIndexLib.find(word);
        if (it == _invertIndexLib.end())
        {
            continue;
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
    // test
    /* for (const auto & [docid, weights] : doc_weight) */
    /* { */
    /*     std::cout << docid << " "; */
    /*     for (auto elem : weights) */
    /*     { */
    /*         std::cout << elem << " "; */
    /*     } */
    /*     std::cout << "\n"; */
    /* } */
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
        std::cout << weight_query[idx] << "\n";
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

    std::cout << "doc_weight.size = " << doc_weight.size() << "\n";
    vector<pair<int, double>> cosins;
    size_t idx = 0;
    /* for (size_t idx = 0; idx < doc_weight.size(); ++idx) */
    for (const auto & [docid, weights] : doc_weight)
    {
        double cos = 0.0;
        double weight_mo = 0.0;
        /* std::cout << "cos is ok" << "\n"; */
        for (size_t i = 0; i < weight_query.size(); ++i)
        {
            /* std::cout << "doc_weight[idx][i] = " << weights[i] << "\n"; */
            cos += (weights[i] * weight_query[i]);
            /* std::cout << "cos is ok" << "\n"; */
            weight_mo += (weight_query[i] * weight_query[i]);
        }
        cosins.push_back({docid, (cos / (doc_weight_mo[idx] * sqrt(weight_mo)))});
        ++idx;
    }

    std::cout << "cosins is ok" << "cosins.size = " << cosins.size() << "\n";
    std::sort(cosins.begin(), cosins.end(), 
                [](const pair<int, double>& a, const pair<int, double>& b){
                     return a.second > b.second;
              });

    // 选取余弦值最大的前10个文章并json化传个客户端
    for (size_t i = 0; i < 10; ++i)
    {
        std::cout << cosins[i].first << " " << cosins[i].second << "\n";
    }
}




