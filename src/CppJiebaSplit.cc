#include "CppJieBaSplit.h"

CppJiebaSplit::CppJiebaSplit(const Configuration * config)
: _config(config)
, _jieba(_config->getConfig().at("dict_path").c_str(), _config->getConfig().at("model_path").c_str(),
         _config->getConfig().at("user_dict_path"), _config->getConfig().at("idf_path"),
         _config->getConfig().at("stop_word_path"))
{

}
/* void CppJiebaSplit::cut(const string& key, vector<string>& words) */
/* { */
/*     _jieba.Cut(key, words, true); */
/* } */

void CppJiebaSplit::cut(const string& key, vector<string>& clearWords, const set<string>& stop_words)
{
    vector<string> words;
    /* _jieba.Cut(key, words, true); */
    _jieba.CutAll(key, words);

    for (auto & word : words)
    {
        if (! word.empty() && (word.back() == '\r' || word.back() == '\n'))
        {
            word.pop_back();
        }
        if (stop_words.find(word) == stop_words.end())
        {
            clearWords.push_back(word);
        }
    }
}

void CppJiebaSplit::cut(const string& key, map<string, int>& clearWords, const set<string>& stop_words)
{
    vector<string> words;
    /* _jieba.Cut(key, words, true); */
    _jieba.CutAll(key, words);
    for (auto & word : words)
    {
        if (!word.empty() && word.back() == '\r' || word.back() == '\n')
        {
            word.pop_back();
        }
        if (stop_words.find(word) == stop_words.end())
        {
            pair<map<string, int>::iterator, bool> p = clearWords.insert({word, 1});
            if (! p.second)
            {
                ++p.first->second;
            }
        }
    }
}
