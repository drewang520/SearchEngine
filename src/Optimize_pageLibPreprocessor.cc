#include "pageLibPreprocessor.h"
#include "tinyxml2.h"
#include <csignal>
#include <cstddef>
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

pageLibPreprocessor::pageLibPreprocessor(const Configuration& config)
: m_config(config)
{

}

void pageLibPreprocessor::buildInvertIndexMap()
{
    string newripepage = m_config.getConfig().at("newripepage");
    std::ifstream ifs(m_config.getConfig().at("newoffset"));
    map<string, int> clearWords;
    map<string, set<int>> docFrequency;
    CppJiebaSplit cppjieba(m_config);
    string line;
    int docid, pos, offset;
    while (std::getline(ifs, line))
    {
        istringstream iss( line);
        iss >> docid >> pos >> offset;
        m_offsetLib[docid] = {pos, offset};
    }
    ifs.close();
    int pageNum = m_offsetLib.size();
    ifs.open(newripepage);
    for (size_t i = 0; i < pageNum; ++i)
    {
        ifs.seekg(m_offsetLib[i + 1].first);
        vector<char> buf(m_offsetLib[i + 1].second + 1);
        ifs.read(buf.data(), m_offsetLib[i + 1].second);
        if (!buf.empty())
        {
            XMLDocument xml;
            XMLError error = xml.Parse(buf.data());        
            if (error == tinyxml2::XML_SUCCESS)
            {
                XMLElement * doc = xml.RootElement();
                string docid = doc->FirstChildElement("docid")->GetText();
                int docid_int = std::stoi(docid);
                string content = doc->FirstChildElement("content")->GetText();
                cppjieba.cut(content, clearWords);
                dealContent(clearWords, docid_int, docFrequency);
                clearWords.clear();
            }
        }
    }
    /* std::cout << "invertIndex.size() = " << m_invertIndexTable.size() << "\n"; */

    double all_weight = 0;
    set<double> weight;
    for (auto & [word, index]: m_invertIndexTable)
    {
        size_t docFre = docFrequency[word].size();
        /* std::cout << "docFre = " << docFre << "\n"; */
        double idf = log2((static_cast<double>(pageNum) + 1.0) / (static_cast<double>(docFre) + 1.0)) + 1.0; 
        /* std::cout << "idf = " << idf << "\n"; */
        for (auto & [docid, weight] : index)
        {
             weight *= idf;
        }
    }

    // weight normalization
    unordered_map<int, double> docNorm2;
    for (const auto & [word, postings] : m_invertIndexTable)
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
    for (auto & [word, postings] : m_invertIndexTable)
    {
        for (auto & [docid, weight] : postings)
        {
            if (docNorm2[docid] > 0)
            {
                weight /= docNorm2[docid];
            }
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
                                    m_invertIndexTable.insert({word, occur_id_weight});
        if (!p.second)
        {
            p.first->second.push_back({docid_int, frequency});
        }
    }
}

void pageLibPreprocessor::store()
{
    ofstream ofs(m_config.getConfig().at("invertIndexTable"));
    for (auto elem : m_invertIndexTable)
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


