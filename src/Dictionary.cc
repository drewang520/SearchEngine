#include "Dictionary.h"
#include "Configuration.h"
#include <fstream>
#include <sstream>
#include <string>
using std::ifstream;
using std::ofstream;
using std::istringstream;

Dictionary::Dictionary(const Configuration * config)
: _config(config)
{
    init();
}

void Dictionary::init()
{
    ifstream ifs1(_config->getConfig().at("dic.dat"));
    ifstream ifs2(_config->getConfig().at("dicIndex.dat"));
    string line;
    while (std::getline(ifs1, line))
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
    
    while (std::getline(ifs2, line))
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
                p.second.insert(std::stoi(word));        
            }
        }
        pair<map<string, set<int>>::iterator, bool> P = _index.insert(p);
    }
}

vector<pair<string, int>> & Dictionary::getDict()
{
    return _dict;
}

map<string, set<int>> & Dictionary::getIndex()
{
    return _index;
}

