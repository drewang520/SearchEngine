#include "../include/cppjieba/Jieba.hpp"
#include "../include/Configuration.h"
#include <ostream>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using std::string;
using std::cout;
using std::ifstream;
using std::istringstream;


void test1()
{
    ifstream ifs("hello.txt");
    /* string line; */
    /* while (std::getline(ifs, line)) */
    /* { */
        /* istringstream isf(line); */
        string doc, title, link, content, docid;
        while ( ifs >> doc >> docid >> title >> link >> content)
        {
            std::cout << "doc: " << doc << "\n" 
                << "docid = " << docid << "\n"
                << "title: " << title << "\n" 
                << "link: " << link << "\n"
                << "content" << content << "\n";
            std::cout << "\n";
        }
    /* } */

}

void test2()
{
    ifstream ifs("hello.txt");
    ifs.seekg(0);
    char * page = new char[3166];
    ifs.read(page, 3166 - 14);
    string onepage(page);
    istringstream iss(onepage);
    string doc, title, link, content, docid;
    while (iss >> doc >> docid >> title >> link >> content)
    {
        std::cout << "doc: " << doc << "\n" 
            << "docid = " << docid << "\n"
            << "title: " << title << "\n" 
            << "link: " << link << "\n"
            << "content" << content << "\n";
    }
}

void test3()
{
    const Configuration * _config = Configuration::createpInstance();
    set<string> stop_words = _config->getStopWords();
    ifstream ifs("../raw_data/module1/yuliao/C3-Art0019.txt");
    std::ofstream ofs("data.txt");
    char buf[65536] = {};
    ifs.read(buf, 65536);
    std::vector<string> words;
    cppjieba::Jieba Jieba(_config->getConfig().at( "dict_path").c_str(),  _config->getConfig().at( "model_path").c_str(),
             _config->getConfig().at( "user_dict_path"),  _config->getConfig().at( "idf_path"),
            _config->getConfig().at("stop_word_path"));
    Jieba.CutAll(buf, words);
     for (auto & word : words)
     {
           if (! word.empty() && (word.back() == '\r' || word.back() == '\n'))
           {       
               word.pop_back();
           }           
           if (stop_words.find( word) == stop_words.end())
           {           
               /* clearWords.push_back(word); */
                ofs << word << " ";
           }       
     }
    ifs.close();
    ofs.close();
}

int main(int argc, char * argv[])
{
    test3();
}

