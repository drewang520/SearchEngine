#include "Configuration.h"
#include "CppJieBaSplit.h"
#include "pageLibPreprocessor.h"
#include "tinyxml2.h"
#include <math.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

using std::ifstream;
using std::ofstream;
using std::istringstream;
using std::pair;

using namespace tinyxml2;

pageLibPreprocessor::pageLibPreprocessor(const Configuration * config)
: _config(config)
{

}

void pageLibPreprocessor::buildInvertIndexMap()
{
    const string newripepage = _config->getConfig().at("newripepage");
    const string newoffset = _config->getConfig().at("newoffset");
    set<string> stopWords = _config->getStopWords();
    map<string, int> clearWords;
    map<string, set<int>> docFrequency;
    CppJiebaSplit cppjieba(_config);

    /* std::cout << "newripepage: " << newripepage << "\n"; */
    XMLDocument xml;
    xml.LoadFile(newripepage.c_str());
    if (xml.ErrorID())
    {
        std::cerr << "LoadFile fail" << "\n";
        return;
    }

    XMLNode * doc = xml.FirstChildElement("doc");
    int pageNum = xml.ChildElementCount();
    std::cout << "pageNum = " << pageNum << "\n";
    while (doc)
    {
        string docid = doc->FirstChildElement("docid")->GetText();
        int docid_int = std::stoi(docid);
        /* std::cout << "docid_int = " << docid_int << "\n"; */
        string content = doc->FirstChildElement("content")->GetText();
        /* std::cout << "no problem CutCLear" << "\n"; */
        cppjieba.cut(content, clearWords, stopWords);
        dealContent(clearWords, docid_int, docFrequency);
        doc = doc->NextSiblingElement("doc");
        clearWords.clear();
    }
    std::cout << "invertIndex.size() = " << _invertIndexTable.size() << "\n";

    double all_weight = 0;
    set<double> weight;
    for (auto & [word, index]: _invertIndexTable)
    {
        size_t docFre = docFrequency[word].size();
        /* std::cout << "docFre = " << docFre << "\n"; */
        double idf = log2(pageNum / (docFre + 1)); 
        /* std::cout << "idf = " << idf << "\n"; */
        for (auto & [docid, weight] : index)
        {
             weight *= idf;
        }
    }

    // weight normalization
    unordered_map<int, double> docNorm2;
    for (const auto & [word, postings] : _invertIndexTable)
    {
        for (const auto & [docid, weight] : postings)
        {
            docNorm2[docid] += weight * weight;
        }
    }
    for (auto & [docid, sum2]: docNorm2)
    {
        sum2 = std::sqrt(sum2);
    }
    for (auto & [word, postings] : _invertIndexTable)
    {
        for (auto & [docid, weight] : postings)
        {
            weight /= docNorm2[docid];
        }
    }
}

void pageLibPreprocessor::dealContent(map<string, int>& clearWords, int & docid_int, 
                                       map<string, set<int>>& docFrequency)
{
    //對於TF，使用map<string, int>獲取單詞在一篇文章的次數
    //對於DF，使用map<string, set<int>>來獲取單詞和在所有文章中的詞頻
    // 遍歷map<string, set<int>>的所有單詞，構造倒排索引

    vector<pair<int, double>> occur_id_weight;
    set<int> docid;
    docid.insert(docid_int);
    for (const auto& [word, frequency] : clearWords)
    {
        occur_id_weight.clear();
        pair<map<string, set<int>>::iterator, bool> p1 =  docFrequency.insert({word, docid}); 
        if (! p1.second)
        {
            p1.first->second.insert(docid_int);
        }
        occur_id_weight.push_back({docid_int, frequency});
        pair<unordered_map<string, vector<pair<int, double>>>::iterator, bool> p = 
                                    _invertIndexTable.insert({word, occur_id_weight});
        if (!p.second)
        {
            p.first->second.push_back({docid_int, frequency});
        }
    }
}

void pageLibPreprocessor::store()
{
    ofstream ofs(_config->getConfig().at("invertIndexTable"));
    for (auto elem : _invertIndexTable)
    {
        ofs << elem.first;
        for (auto sets : elem.second)
        {
            ofs << " " << sets.first << " " << sets.second << " ";
        }
        ofs << "\n";
    }
    ofs.close();
}




