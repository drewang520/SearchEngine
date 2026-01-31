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
    ifstream ifs(_config->getConfig().at("dic.dat"));
    string line;
    string word;
    int num;
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        iss >> word >> num;
        _dict.push_back(std::make_pair(word, num));
    }
    ifs.close();
    ifs.open(_config->getConfig().at("dicIndex.dat"));
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        iss >> word;
        while (iss >> num)
        {
            _index[word].insert(num);
        }
    }
    ifs.close();
}

vector<pair<string, int>> & Dictionary::getDict()
{
    return _dict;
}

map<string, set<int>> & Dictionary::getIndex()
{
    return _index;
}

