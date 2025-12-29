#include "pageLibPreprocessor.h"
#include "tinyxml2.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

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
    ifstream ifs2(newoffset);
    string line, docid, pos, offset;
    pair<int, int> pos_len;
    while (std::getline(ifs2, line))
    {
        istringstream iss(line);
        while (iss >> docid >> pos >> offset)
        {
            pos_len = {std::stoi(pos), std::stoi(offset)};
            _offsetLib.insert({std::stoi(docid), pos_len});
        }
    }
    
    /* const char * dict_path = "../raw_data/module1/dict/jieba.dict.utf8"; */
    /* const char * model_path = "../raw_data/module1/dict/hmm_model.utf8"; */
    /* const char * user_dict_path = "../raw_data/module1/dict/user.dict.utf8"; */
    /* const char * idf_path = "../raw_data/module1/dict/idf.utf8"; */
    /* const char * stop_word_path = "../raw_data/module1/dict/stop_words.utf8"; */
    /* cppjieba::Jieba jieba(dict_path, model_path, user_dict_path, idf_path, stop_word_path); */
    /* /1* Jieba jieba; *1/ */
    /* vector<string> words; */
    /* set<string> _stopWords; */
    /* ifstream ifs3(stop_word_path); */
    /* while (std::getline(ifs3, line)) */
    /* { */
    /*     if (!line.empty() && line.back() == '\r') */
    /*     { */
    /*         line.pop_back(); */
    /*     } */
    /*     _stopWords.insert(line); */
    /* } */

    std::cout << "newripepage: " << newripepage << "\n";
    XMLDocument xml;
    xml.LoadFile(newripepage.c_str());
    if (xml.ErrorID())
    {
        std::cerr << "LoadFile fail" << "\n";
        return;
    }

    XMLNode * doc = xml.FirstChildElement("doc");
    int pageNum = xml.ChildElementCount();
    /* std::cout << "pageNum = " << pageNum << "\n"; */
    while (doc)
    {
        string docid = doc->FirstChildElement("docid")->GetText();
        int docid_int = std::stoi(docid);
        string content = doc->FirstChildElement("content")->GetText();

        doc = doc->NextSiblingElement("doc");
    }

}

void pageLibPreprocessor::dealContent(vector<string>& words, const cppjieba::Jieba& jieba, 
                    int & docid_int, const string& content, set<string>& stop_words)
{
    //對於TF，使用map<string, int>獲取單詞在一篇文章的次數
    //對於DF，使用map<string, set<int>>來獲取單詞和在所有文章中的詞頻
    // 遍歷map<string, set<int>>的所有單詞，構造倒排索引
    jieba.Cut(content, words, true);
    set<pair<int, double>> occur_id_weight;
    pair<int, double> id_weight;
    double weight = 1;
    for (auto word : words)
    {
        if (stop_words.find(word) == stop_words.end())
        {
            id_weight = {docid_int, weight};
            pair<set<pair<int, double>>::iterator, bool> p = occur_id_weight.insert(id_weight);
            if (! p.second)
            {
                /* ++p.first->second; */
            }
            _invertIndexTable.insert({word, occur_id_weight});                        
        }
    }
}


void pageLibPreprocessor::store()
{

}
