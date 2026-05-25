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
    /* ifstream ifs2(newoffset); */
    /* string line, docid, pos, offset; */
    /* pair<int, int> pos_len; */
    /* while (std::getline(ifs2, line)) */
    /* { */
    /*     istringstream iss(line); */
    /*     while (iss >> docid >> pos >> offset) */
    /*     { */
    /*         pos_len = {std::stoi(pos), std::stoi(offset)}; */
    /*         m_offsetLib.insert({std::stoi(docid), pos_len}); */
    /*     } */
    /* } */
    
    const char * dict_path = "../raw_data/module1/dict/jieba.dict.utf8";
    const char * model_path = "../raw_data/module1/dict/hmm_model.utf8";
    const char * user_dict_path = "../raw_data/module1/dict/user.dict.utf8";
    const char * idf_path = "../raw_data/module1/dict/idf.utf8";
    const char * stop_word_path = "../raw_data/module1/dict/stop_words.utf8";
    cppjieba::Jieba jieba(dict_path, model_path, user_dict_path, idf_path, stop_word_path);

    vector<string> words;
    set<string> m_stopWords;
    vector<string> clearWords;
    ifstream ifs3(stop_word_path);
    string line;
    while (std::getline(ifs3, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        m_stopWords.insert(line);
    }

    std::cout << "newripepage: " << newripepage << "\n";
    XMLDocument xml;
    
    xml.LoadFile(newripepage.c_str());
    if (xml.ErrorID())
    {
        std::cerr << "LoadFile fail" << "\n";
        return;
    }

    vector<map<string, int>> wordFrequency;
    map<string, set<int>> docFrequency;

    XMLNode * doc = xml.FirstChildElement("doc");
    int pageNum = xml.ChildElementCount();
    /* std::cout << "pageNum = " << pageNum << "\n"; */
    while (doc)
    {
        string docid = doc->FirstChildElement("docid")->GetText();
        int docid_int = std::stoi(docid);
        std::cout << "docid_int = " << docid_int << "\n";
        string content = doc->FirstChildElement("content")->GetText();
        /* std::cout << "no problem CutCLear" << "\n"; */
        dealContent(CutClear(content, jieba, words, clearWords,m_stopWords),
                        docid_int, wordFrequency, docFrequency);
        doc = doc->NextSiblingElement("doc");
        words.clear();
        clearWords.clear();
    }
    /* for (auto elem : docFrequency) */
    /* { */
    /*     std::cout << "elem.string = " << elem.first << " sets: "; */
    /*     for (auto sets: elem.second) */
    /*     { */
    /*         std::cout <<  sets << " "; */ 
    /*     } */
    /*     std::cout << "\n"; */
    /* } */
    set<pair<int, float>> occur_id_weight;
    pair<int, float> id_weight;
    float weight = 1;
    unordered_map<string, set<pair<int, float>>> invertTmp;
    for (size_t idx = 0; idx < wordFrequency.size(); ++idx)
    {
        for (auto elem : wordFrequency[idx])
        {
            string word = elem.first;
            size_t termFre = elem.second;
            size_t docFre = docFrequency[word].size();
            /* float idf = log2(pageNum / (docFre + 1)); */ 
            /* float idf = 1.0; */ 
            /* weight = termFre * idf; */
            weight = termFre;
            id_weight = {idx + 1, weight};
            occur_id_weight.insert(id_weight);
            pair<unordered_map<string, set<pair<int, float>>>::iterator, bool> p = 
                    invertTmp.insert({word, occur_id_weight});
            if (! p.second)
            {
                p.first->second.insert(id_weight);
            }
        }
    }
    // weight normalization
    float all_weight = 0;
    float normalize_weight;
    occur_id_weight.clear();
    for (auto elem : invertTmp)
    {
        for (auto sets : elem.second)
        {
            all_weight += pow(sets.second, 2);
        }
        for (auto sets : elem.second)
        {
            normalize_weight = sets.second / sqrt(all_weight);
            id_weight = {sets.first, normalize_weight};
            occur_id_weight.insert(id_weight);
            pair<unordered_map<string, set<pair<int, float>>>::iterator, bool> p = 
                    m_invertIndexTable.insert({elem.first, occur_id_weight});
            if (! p.second)
            {
                p.first->second.insert(id_weight);
            }
        }
    }
}

vector<string>& pageLibPreprocessor::CutClear(const string& content, const cppjieba::Jieba& jieba,
                                             vector<string>& words, vector<string>& clearWords, set<string>& stop_words)
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
            clearWords.push_back(word);
        }
    }
    return clearWords;
}

void pageLibPreprocessor::dealContent(vector<string>& clearWords, int & docid_int, vector<map<string, int>>& wordFrequency,
                                       map<string, set<int>>& docFrequency)
{
    //對於TF，使用map<string, int>獲取單詞在一篇文章的次數
    //對於DF，使用map<string, set<int>>來獲取單詞和在所有文章中的詞頻
    // 遍歷map<string, set<int>>的所有單詞，構造倒排索引

    set<int> docid;
    docid.insert(docid_int);
    map<string, int> wordFre;
    for (auto word : clearWords)
    {
        pair<map<string, set<int>>::iterator, bool> p1 =  docFrequency.insert({word, docid}); 
        if (! p1.second)
        {
            if (p1.first->second.find(docid_int) == p1.first->second.end())
            {
                p1.first->second.insert(docid_int);
            }
        }
        /* std::cout << "docFrequency is ok" << "\n"; */
        pair<map<string, int>::iterator, bool> p2 = wordFre.insert({word, 1});
        if (! p2.second)
        {
            ++p2.first->second;
        }
    }
    wordFrequency.push_back(wordFre);
}

void pageLibPreprocessor::store(const string& saveInvertIndex)
{
    ofstream ofs("saveInvertIndex");
    for (auto elem : m_invertIndexTable)
    {
        ofs << elem.first << " ";
        for (auto sets : elem.second)
        {
            ofs << sets.first << " " << sets.second << " ";
        }
        ofs << "\n";
    }
    ofs.close();
}




