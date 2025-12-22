#include "DicProducer.h"
#include <cstddef>
#include <sys/types.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using std::istringstream;
using std::ifstream;
using std::ofstream;

DicProducer::DicProducer(string filename)
{
    DIR * pdir = opendir(filename.c_str());
    string file;
    _files.reserve(50);
    if (pdir == nullptr)
    {
        //该路径为文件
        ifstream ifs(filename);
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
        // 该路径为目录
        struct dirent * pdirent;        
        while ((pdirent = readdir(pdir)) != nullptr)
        {
            string fileName = filename + pdirent->d_name;
            std::cout << fileName << "\n";
            ifstream ifs(fileName);
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
        std::cout << "finish\n";
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

void DicProducer::buildEnDict()
{
    //加载并清洗英文停用词
    set<string> _stopWords;
    ifstream ifs("../raw_data/module1/yuliao/stop-words-list/stop_words_eng.txt");
    string line;
    while (std::getline(ifs, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        _stopWords.insert(line);
    }

    // 构造英文词典
    for (auto file : _files)
    {
        istringstream isf(file);
        string word;
        while(isf >> word)
        {
            if (_stopWords.find(word) == _stopWords.end())
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
    ifs.close();
}

void DicProducer::buildCnDict()
{

}

void DicProducer::createIndex()
{
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

