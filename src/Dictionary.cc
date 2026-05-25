#include "Dictionary.h"
#include "Configuration.h"
#include <fstream>
#include <sstream>
#include <string>
using std::ifstream;
using std::ofstream;
using std::istringstream;

Dictionary::Dictionary(const Configuration& config)
: m_config(config)
{
    init();
}

void Dictionary::init()
{
    ifstream ifs(m_config.getConfig().at("dic.dat"));
    string line;
    string word;
    int num;
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        iss >> word >> num;
        m_dict.push_back(std::make_pair(word, num));
    }
    ifs.close();
    ifs.open(m_config.getConfig().at("dicIndex.dat"));
    while (std::getline(ifs, line))
    {
        istringstream iss(line);
        iss >> word;
        while (iss >> num)
        {
            m_index[word].insert(num);
        }
    }
    ifs.close();
}

vector<pair<string, int>> & Dictionary::getDict()
{
    return m_dict;
}

map<string, set<int>> & Dictionary::getIndex()
{
    return m_index;
}

const vector<pair<string, int>> & Dictionary::getDict() const
{
    return m_dict;
}

const map<string, set<int>> & Dictionary::getIndex() const
{
    return m_index;
}
