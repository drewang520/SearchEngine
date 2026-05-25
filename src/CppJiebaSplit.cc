#include "CppJieBaSplit.h"

CppJiebaSplit::CppJiebaSplit(const Configuration& config)
: m_config(config)
, m_stopWords(m_config.getStopWords())
, m_jieba(m_config.getConfig().at("dict_path").c_str(), m_config.getConfig().at("model_path").c_str(),
         m_config.getConfig().at("user_dict_path"), m_config.getConfig().at("idf_path"),
         m_config.getConfig().at("stop_word_path"))
{

}

void CppJiebaSplit::cut(const string& key, vector<string>& clearWords) const
{
    vector<string> words;
    /* m_jieba.Cut(key, words, true); */
    m_jieba.CutAll(key, words);

    for (auto & word : words)
    {
        if (! word.empty() && (word.back() == '\r' || word.back() == '\n'))
        {
            word.pop_back();
        }
        if (!word.empty() && m_stopWords.find(word) == m_stopWords.end())
        {
            clearWords.push_back(word);
        }
    }
}

void CppJiebaSplit::cut(const string& key, map<string, int>& clearWords) const
{
    vector<string> words;
    /* m_jieba.Cut(key, words, true); */
    m_jieba.CutAll(key, words);
    for (auto & word : words)
    {
        if (!word.empty() && (word.back() == '\r' || word.back() == '\n'))
        {
            word.pop_back();
        }
        if (!word.empty() && m_stopWords.find(word) == m_stopWords.end())
        {
            pair<map<string, int>::iterator, bool> p = clearWords.insert({word, 1});
            if (! p.second)
            {
                ++p.first->second;
            }
        }
    }
}
