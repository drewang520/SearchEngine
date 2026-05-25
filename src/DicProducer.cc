#include "DicProducer.h"
#include "CppJieBaSplit.h"
#include <climits>
#include <cstddef>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

using std::istringstream;
using std::ifstream;
using std::ofstream;

DicProducer::DicProducer(const string& filename, const Configuration& config)
: m_config(config)
{
    set<string> stopWords = m_config.getStopWords();
    DIR * pdir = opendir(filename.c_str());
    m_files.reserve(200);
    if (pdir == nullptr)
    {
        m_files.clear();
        //该路径为文件,处理英文
        ifstream ifs(filename);
        string file;
        string line;
        while (std::getline(ifs, line))
        {
            string word;
            istringstream isf(line);
            while (isf >> word)
            {
                string newWord = dealWord(word);
                file.append(newWord + ' ');
            }
        }
        m_files.push_back(file);
    }
    else
    {
        // 该路径为目录, 处理中文
        m_files.clear();
        struct dirent * pdirent;        
        vector<string> clearWords;
        CppJiebaSplit cppjieba(m_config);

        char * buf = new char[65550]();
        while ((pdirent = readdir(pdir)) != nullptr)
        {
            clearWords.clear();
            bzero(buf, 65550);
            string fileName = filename + '/' + pdirent->d_name;
            std::cout << fileName << "\n";
            ifstream ifs(fileName);
            ifs.read(buf, 65550);
            cppjieba.cut(buf, clearWords);
            if (!clearWords.empty())
            {
                for (size_t i = 0; i < clearWords.size(); ++i)
                {
                    m_files.push_back(clearWords[i]);
                }
            }
            /* std::cout << "buf.strlen = " << strlen(buf) << "\n"; */
            /* CnDispatch(buf); */
            /* std::cout << "_file.size() = " << m_files.size() << "\n"; */
        }
        delete [] buf;
        /* std::cout << "_file.size() = " << m_files.size() << "\n"; */
        /* std::cout << "finish\n"; */
    }
}


string DicProducer::dealWord(string word)
{
    for (auto &ch : word)
    {
        // 如果是标点符号就转为空格
        if (!isalpha(ch))
        {
            ch = ' ';
        }
        // 如果是大写就转为小写
        else if (isupper(ch))
        {
            ch = tolower(ch);
        }
    }
    return word;
}

void DicProducer::printFile() const
{
    std::cout << "FILE: " << "\n";
    for (auto file : m_files)
    {
        std::cout << file;
    }
}

void DicProducer::buildEnDict(const set<string>& stopWords)
{
    m_dict.clear();

    // 构造英文词典
    for (auto file : m_files)
    {
        istringstream isf(file);
        string word;
        while(isf >> word)
        {
            if (stopWords.find(word) == stopWords.end())
            {
                pair<map<string, int>::iterator, bool> p 
                                                = m_dict.insert({word, 1});
                if (! p.second)
                {
                    ++p.first->second;
                }
            }
        }
    }
}

void DicProducer::buildCnDict(const set<string>& stopWords)
{
    m_dict.clear();
    
    //构造中文词典
    for (auto word : m_files)
    {
        istringstream isf(word);
        string Word;
        while (isf >> Word)
        {
             pair<map<string, int>::iterator, bool> p
                                            = m_dict.insert({Word, 1});
             if (!p.second)
             {
                ++p.first->second;
             }
        }
    }
}

void DicProducer::createEnIndex()
{
    m_index.clear();
    m_dict2.clear();
    for (auto pairs : m_dict)
    {
        m_dict2.push_back(pairs);
    }

    for (size_t index = 0; index < m_dict2.size(); ++index)
    {
        string word = m_dict2[index].first;
        set<int> _setNum = {};
        _setNum.insert(index);
        for (size_t i = 0; i < word.size(); ++i)
        {
            pair<map<string, set<int>>::iterator, bool> p
                                                = m_index.insert({string(1, word[i]), _setNum});
            if (!p.second)
            {
                p.first->second.insert(index);
            }
        }
    }
}

void DicProducer::createCnIndex()
{
    m_index.clear();
    m_dict2.clear();
    for (auto pairs : m_dict)
    {
        m_dict2.push_back(pairs);
    }

    for (size_t index = 0; index < m_dict2.size(); ++index)
    {
        string word = m_dict2[index].first;
        set<int> _setNum = {};
        _setNum.insert(index);
        int length = 1;
        for (size_t i = 0; i < word.size();)
        {
            unsigned char c = word[i];
            if (c >= 0xF0) length = 4;
            else if (c >= 0xE0) length = 3;
            else if (c >= 0xC0) length = 2;

            string Word = word.substr(i, length);
            pair<map<string, set<int>>::iterator, bool> p
                                                = m_index.insert({ Word, _setNum});
            if (!p.second)
            {
                p.first->second.insert(index);
            }
            i += length;
        }
    }
}

void DicProducer::storeDict(string savefile)
{
    ofstream ofs(savefile);
    for (auto it: m_dict)
    {
        ofs << it.first << " " << it.second << "\n";
    }
    ofs.close();
}

void DicProducer::storeIndex(string savefile)
{
    ofstream ofs(savefile);
    for (auto it: m_index)
    {
        ofs << it.first << " "; 
        for (auto sets: it.second)
        {
            ofs << sets << " ";
        }
        ofs << "\n";
    }
    ofs.close();
}

void DicProducer::storeAllDict()
{
    ofstream ofs(m_config.getConfig().at("dic.dat"));
    for (const auto& it: m_dict2)
    {
        ofs << it.first << " " << it.second << "\n";
    }
    ofs.close();
    
}

void DicProducer::storeALlIndex()
{
    ofstream ofs(m_config.getConfig().at("dicIndex.dat"));
    for (const auto& [word, indexs]: m_index)
    {
        ofs << word << " "; 
        for (const auto& index : indexs)
        {
            ofs << index << " ";
        }
        ofs << "\n";
    }
    ofs.close();
}

void DicProducer::buildDictAndIndex()
{
    m_dict2.clear();
    m_index.clear();
    string dicEn_path = m_config.getConfig().at("dicEn.dat"); //"../data/dicEn.dat";
    string dicCn_path = m_config.getConfig().at("dicCn.dat"); //../data/dicCn.dat";
    string EnIndex_path = m_config.getConfig().at("dicindexEn.dat"); //"../data/dicindexEn.dat";
    string CnIndex_path = m_config.getConfig().at("dicindexCn.dat");// "../data/dicindexCn.dat";
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
            m_dict2.push_back(p);
        }
        if (path == dicCn_path)
        {
            Cn_length += m_dict2.size();
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
            pair<map<string, set<int>>::iterator, bool> P = m_index.insert(p);
        }
    }

}
