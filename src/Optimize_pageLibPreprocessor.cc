#include "pageLibPreprocessor.h"
#include "cppjieba/Jieba.hpp"
#include "tinyxml2.h"
#include <math.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

using std::ifstream;
using std::ofstream;
using std::istringstream;
using std::pair;

using namespace tinyxml2;

pageLibPreprocessor::pageLibPreprocessor()
{

}

void pageLibPreprocessor::buildInvertIndexMap(const string& newripepage, const string& newoffset)
{
    ifstream ifs(newripepage);
    
    const char * dict_path = "../raw_data/module1/dict/jieba.dict.utf8";
    const char * model_path = "../raw_data/module1/dict/hmm_model.utf8";
    const char * user_dict_path = "../raw_data/module1/dict/user.dict.utf8";
    const char * idf_path = "../raw_data/module1/dict/idf.utf8";
    const char * stop_word_path = "../raw_data/module1/dict/stop_words.utf8";
    cppjieba::Jieba jieba(dict_path, model_path, user_dict_path, idf_path, stop_word_path);

    vector<string> words;
    set<string> _stopWords;
    map<string, int> clearWords;
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

    std::cout << "newripepage: " << newripepage << "\n";
    XMLDocument xml;
    
    xml.LoadFile(newripepage.c_str());
    if (xml.ErrorID())
    {
        std::cerr << "LoadFile fail" << "\n";
        return;
    }

    map<string, set<int>> docFrequency;

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
        dealContent(CutClear(content, jieba, words, clearWords,_stopWords),
                        docid_int, docFrequency);
        doc = doc->NextSiblingElement("doc");
        words.clear();
        clearWords.clear();
    }
    std::cout << "invertIndex.size() = " << _invertIndexTable.size() << "\n";

    double all_weight = 0;
    set<double> weight;
    for (auto &elem : _invertIndexTable)
    {
        size_t docFre = docFrequency[elem.first].size();
        /* std::cout << "docFre = " << docFre << "\n"; */
        double idf = log2(pageNum / (docFre + 1)); 
        /* std::cout << "idf = " << idf << "\n"; */
        for (auto & vec : elem.second)
        {
            vec.second *= idf;
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


map<string, int>& pageLibPreprocessor::CutClear(const string& content, const cppjieba::Jieba& jieba,
                                             vector<string>& words, map<string, int>& clearWords, set<string>& stop_words)
{
    vector<string> tmp;
    jieba.Cut(content, words, true);
    for (size_t index = 0; index < words.size(); ++index)
    {
        if (!words[index].empty() && words[index].back() == '\r' || words[index].back() == '\n')
        {
            words[index].pop_back();
        }
        tmp.push_back(words[index]);
    }
    for (auto word : tmp)
    {
        if (stop_words.find(word) == stop_words.end())
        {
            pair<map<string, int>::iterator, bool> p = clearWords.insert({word, 1});
            if (! p.second)
            {
                ++p.first->second;
            }
        }
    }
    /* for (auto elem : clearWords) */
    /* { */
    /*     std::cout << "string: " << elem.first << "count: " << elem.second << "\n"; */
    /* } */
    return clearWords;
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
    for (auto word : clearWords)
    {
        occur_id_weight.clear();
        pair<map<string, set<int>>::iterator, bool> p1 =  docFrequency.insert({word.first, docid}); 
        if (! p1.second)
        {
            p1.first->second.insert(docid_int);
        }
        occur_id_weight.push_back({docid_int, word.second});
        pair<unordered_map<string, vector<pair<int, double>>>::iterator, bool> p = 
                                    _invertIndexTable.insert({word.first, occur_id_weight});
        if (!p.second)
        {
            p.first->second.push_back({docid_int, word.second});
        }
    }
}

void pageLibPreprocessor::store(const string& saveInvertIndex)
{
    ofstream ofs(saveInvertIndex);
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




