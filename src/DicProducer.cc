#include "DicProducer.h"
#include "Configuration.h"
#include "CppJieBaSplit.h"
#include <climits>
#include <cstddef>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using std::istringstream;
using std::ifstream;
using std::ofstream;

DicProducer::DicProducer(const string& filename, const Configuration * pInstance)
: _config(pInstance)
{
    DIR * pdir = opendir(filename.c_str());
    _files.reserve(200);
    if (pdir == nullptr)
    {
        _files.clear();
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
        _files.push_back(file);
    }
    else
    {
        // 该路径为目录, 处理中文
        _files.clear();
        struct dirent * pdirent;        
        vector<string> words;
        CppJiebaSplit cppjieba(_config);

        char * buf = new char[65550]();
        while ((pdirent = readdir(pdir)) != nullptr)
        {
            words.clear();
            bzero(buf, 65550);
            string fileName = filename + '/' + pdirent->d_name;
            std::cout << fileName << "\n";
            ifstream ifs(fileName);
            ifs.read(buf, 65550);
            cppjieba.cut(buf, words);
            if (!words.empty())
            {
                for (size_t i = 0; i < words.size(); ++i)
                {
                    _files.push_back(words[i]);
                }
            }
            /* std::cout << "buf.strlen = " << strlen(buf) << "\n"; */
            /* CnDispatch(buf); */
            /* std::cout << "_file.size() = " << _files.size() << "\n"; */
        }
        delete [] buf;
        /* std::cout << "_file.size() = " << _files.size() << "\n"; */
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
    for (auto file : _files)
    {
        std::cout << file;
    }
}

void DicProducer::buildEnDict(const set<string>& stopWords)
{
    _dict.clear();

    // 构造英文词典
    for (auto file : _files)
    {
        istringstream isf(file);
        string word;
        while(isf >> word)
        {
            if (stopWords.find(word) == stopWords.end())
            {
                pair<map<string, int>::iterator, bool> p 
                                                = _dict.insert({word, 1});
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
    _dict.clear();

    //清洗_files中的\r \n
    vector<string> _clearDic;
    for (size_t index = 0; index < _files.size(); ++index)
    {
        if (!_files[index].empty() && _files[index].back() == '\r' || _files[index].back() == '\n')
        {
            _files[index].pop_back();
        }
        _clearDic.push_back(_files[index]);
    }
    
    //构造中文词典
    for (auto word : _clearDic)
    {
        istringstream isf(word);
        string Word;
        while (isf >> Word)
        {
            if (stopWords.find(Word) == stopWords.end())
            {
                pair<map<string, int>::iterator, bool> p
                                                = _dict.insert({Word, 1});
                if (!p.second)
                {
                    ++p.first->second;
                }
            }
        }
    }
}

void DicProducer::createEnIndex()
{
    _index.clear();
    _dict2.clear();
    for (auto pairs : _dict)
    {
        _dict2.push_back(pairs);
    }

    for (size_t index = 0; index < _dict2.size(); ++index)
    {
        string word = _dict2[index].first;
        set<int> _setNum = {};
        _setNum.insert(index);
        for (size_t i = 0; i < word.size(); ++i)
        {
            pair<map<string, set<int>>::iterator, bool> p
                                                = _index.insert({string(1, word[i]), _setNum});
            if (!p.second)
            {
                p.first->second.insert(index);
            }
        }
    }
}

void DicProducer::createCnIndex()
{
    _index.clear();
    _dict2.clear();
    for (auto pairs : _dict)
    {
        _dict2.push_back(pairs);
    }

    for (size_t index = 0; index < _dict2.size(); ++index)
    {
        string word = _dict2[index].first;
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
                                                = _index.insert({ Word, _setNum});
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
    for (auto it: _dict)
    {
        ofs << it.first << " " << it.second << "\n";
    }
    ofs.close();
}

void DicProducer::storeIndex(string savefile)
{
    ofstream ofs(savefile);
    for (auto it: _index)
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
    ofstream ofs( _config->getConfig().at("dic.dat"));
    for (const auto& it: _dict2)
    {
        ofs << it.first << " " << it.second << "\n";
    }
    ofs.close();
    
}

void DicProducer::storeALlIndex()
{
    ofstream ofs(_config->getConfig().at("dicIndex.dat"));
    for (const auto& [word, indexs]: _index)
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
    _dict2.clear();
    _index.clear();
    string dicEn_path = _config->getConfig().at("dicEn.dat"); //"../data/dicEn.dat";
    string dicCn_path = _config->getConfig().at("dicCn.dat"); //../data/dicCn.dat";
    string EnIndex_path = _config->getConfig().at("dicindexEn.dat"); //"../data/dicindexEn.dat";
    string CnIndex_path = _config->getConfig().at("dicindexCn.dat");// "../data/dicindexCn.dat";
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
            _dict2.push_back(p);
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
