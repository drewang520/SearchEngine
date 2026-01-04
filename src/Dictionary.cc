#include "Dictionary.h"
#include "Configuration.h"
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

using std::ifstream;
using std::ofstream;
using std::istringstream;

Dictionary::Dictionary(Configuration * config)
: _config(config)
{
    init();
}

void Dictionary::init()
{
    string dicEn_path = _config->getConfig()["dicEn.dat"]; //"../data/dicEn.dat";
    string dicCn_path = _config->getConfig()["dicCn.dat"]; //../data/dicCn.dat";
    string EnIndex_path = _config->getConfig()["dicindexEn.dat"]; //"../data/dicindexEn.dat";
    string CnIndex_path = _config->getConfig()["dicindexCn.dat"];// "../data/dicindexCn.dat";
    vector<string> dic_path = {dicCn_path, dicEn_path};
    vector<string> Index_path = {CnIndex_path, EnIndex_path};

    size_t Cn_length = 0;
    for (auto path : dic_path)
    {
        ifstream ifs(path);
        string line;
        while (std::getline(ifs, line))
        {
            istringstream ssf(line);
            string word;
            int i = 0;
            pair<string, int> p;
            while (ssf >> word)
            {
                if (i == 0)
                {
                    p.first = word;
                    i = 1;
                }
                else 
                {
                    p.second = std::stoi(word);        
                }
            }
            _dict.push_back(p);
        }
        /* std::cout << "_dict.size(): " << _dict.size() << "\n"; */
        if (path == dicCn_path)
        {
            Cn_length += _dict.size();
            /* std::cout << "Cn_length = " << Cn_length << "\n"; */
        }
    }

    for (auto path : Index_path)
    {
        ifstream ifs(path);
        string line;
        while (std::getline(ifs, line))
        {
            istringstream ssf(line);
            string word;
            int i = 0;
            pair<string, set<int>> p;
            while (ssf >> word)
            {
                if (i == 0)
                {
                    p.first = word;
                    i = 1;
                }
                else 
                {
                    if (path == EnIndex_path)
                    {
                        p.second.insert(std::stoi(word) + Cn_length);        
                    }
                    else 
                    {
                        p.second.insert(std::stoi(word));        
                    }
                }
            }
            pair<map<string, set<int>>::iterator, bool> P = _index.insert(p);
            if (! P.second)
            {
                for (auto elem : p.second)
                {
                    P.first->second.insert(elem);
                }
            }
        }
    }
}

vector<pair<string, int>> & Dictionary::getDict()
{
    /* ofstream ofs( "../data/dic.dat"); */
    /* for (auto it: _dict) */
    /* { */
    /*     ofs << it.first << " " << it.second << "\n"; */
    /* } */
    /* ofs.close(); */
    
    return _dict;
}

map<string, set<int>> & Dictionary::getIndex()
{
    /* ofstream ofs("../data/dicIndex.dat"); */
    /* for (auto it: _index) */
    /* { */
    /*     ofs << it.first << " "; */ 
    /*     for (auto sets: it.second) */
    /*     { */
    /*         ofs << sets << " "; */
    /*     } */
    /*     ofs << "\n"; */
    /* } */
    /* ofs.close(); */
    return _index;
}

