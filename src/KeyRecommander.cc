#include "KeyRecommander.h"
#include "ProtocolParser.h"
#include "CacheManager.h"
#include "Logger.h"
#include <iostream>
#include <mutex>
#include <algorithm>
#include <stdexcept>
#include <vector>

using namespace Protocol;

namespace
{
const Dictionary& getSharedDictionary(const Configuration& config)
{
    static Dictionary dictionary(config);
    return dictionary;
}

std::vector<std::string> splitUtf8Chars(const std::string& word)
{
    std::vector<std::string> chars;
    for (size_t idx = 0; idx < word.size(); )
    {
        unsigned char ch = word[idx];
        size_t nBytes = 1;
        if (ch >= 0xF0)
        {
            nBytes = 4;
        }
        else if (ch >= 0xE0)
        {
            nBytes = 3;
        }
        else if (ch >= 0xC0)
        {
            nBytes = 2;
        }
        chars.push_back(word.substr(idx, nBytes));
        idx += nBytes;
    }
    return chars;
}
}

KeyRecommander::KeyRecommander(string queryWords, const Configuration& config)
: m_queryWords(queryWords)
, m_dict(getSharedDictionary(config))
, m_config(config)
{

}

string KeyRecommander::startQuery(int cacheID)
{
    LOG_DEBUG("key recommender start query=" + m_queryWords
              + " cacheID=" + std::to_string(cacheID));
    if (cacheID < 0)
    {
        throw std::out_of_range("KeyRecommander::startQuery invalid cacheID");
    }

    CacheManager& cacheManager = CacheManager::createCacheManger();
    const size_t cacheIndex = static_cast<size_t>(cacheID);
    json j;
    {
        std::lock_guard<std::mutex> lock(cacheManager.getMutex(cacheIndex));
        auto &cache = cacheManager.getCache(cacheIndex);
        if (cache.get(m_queryWords, j))
        {
            LOG_DEBUG("key recommender cache hit query=" + m_queryWords);
            return ProtocolParser::JsonToString(j);
        }
    }

    LOG_DEBUG("key recommender cache miss query=" + m_queryWords);
    j = ProtocolParser::vecToJson(doQuery());

    {
        std::lock_guard<std::mutex> lock(cacheManager.getMutex(cacheIndex));
        auto &cache = cacheManager.getCache(cacheIndex);
        json cached;
        if (cache.get(m_queryWords, cached))
        {
            LOG_DEBUG("key recommender cache filled by another worker query=" + m_queryWords);
            return ProtocolParser::JsonToString(cached);
        }
        cache.addElem(m_queryWords, j);
        LOG_DEBUG("key recommender cache updated query=" + m_queryWords);
    }

    return ProtocolParser::JsonToString(j);
}

vector<string> KeyRecommander::doQuery()
{
    const auto& index = m_dict.getIndex();
    const auto& dict = m_dict.getDict();
    vector<string> queryResult = {};
    map<string, int> candidataWords = {};
    for (const auto& single_ch : splitUtf8Chars(m_queryWords))
    {
        queryIndex(candidataWords, index, dict, single_ch);
    }
    candidataSort(candidataWords, queryResult);
    LOG_INFO("key recommender query=" + m_queryWords
             + " candidates=" + std::to_string(candidataWords.size())
             + " resultCount=" + std::to_string(queryResult.size()));
    return queryResult;
}

void KeyRecommander::queryIndex(map<string, int>& candidataWords, const map<string, set<int>>& index, 
                                    const vector<pair<string, int>>& dict, const string& single_ch) const
{
    auto it = index.find(single_ch);
    if (it != index.end())
    {
        for (auto elem : it->second)
        {
            if (elem >= 0 && static_cast<size_t>(elem) < dict.size())
            {
                candidataWords[dict[elem].first] = dict[elem].second;
            }
        }
    }
}

void KeyRecommander::candidataSort(const map<string, int>& candidataWords, vector<string>& queryResult) const
{
    priority_queue<CandidateResult, vector<CandidateResult>, CompareHot> prique;
    for (const auto& [word, frequency] : candidataWords)
    {
        CandidateResult canresult;
        canresult.m_word = word;
        canresult.m_freq = frequency;
        canresult.m_dist = editDistance(canresult.m_word, m_queryWords);
        prique.push(canresult);         
    }

    size_t candidataNum = 0;
    const size_t resultLimit = static_cast<size_t>(std::stoi(m_config.getConfig().at("candidataNum")));
    while (!prique.empty() && candidataNum != resultLimit)
    {
        queryResult.push_back(prique.top().m_word);
        prique.pop();
        ++candidataNum;
    }
}

size_t nBytesCode(const char ch)
{
	if(ch & (1 << 7))
	{
		int nBytes = 1;
		for(int idx = 0; idx != 6; ++idx)
		{
			if(ch & (1 << (6 - idx)))
			{
				++nBytes;
			}
			else
				break;
		}
		return nBytes;
	}
	return 1;
}

std::size_t length(const std::string &str)
{
	std::size_t ilen = 0;
	for(std::size_t idx = 0; idx != str.size(); ++idx)
	{
		int nBytes = nBytesCode(str[idx]);
		idx += (nBytes - 1);
		++ilen;
	}
	return ilen;
}

int triple_min(const int &a, const int &b, const int &c)
{
	return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

int KeyRecommander::editDistance(const std::string & lhs, const std::string &rhs) const
{//计算最小编辑距离-包括处理中英文
    std::vector<std::string> lhsChars = splitUtf8Chars(lhs);
    std::vector<std::string> rhsChars = splitUtf8Chars(rhs);
	size_t lhs_len = lhsChars.size();
	size_t rhs_len = rhsChars.size();
    std::vector<std::vector<int>> editDist(lhs_len + 1, std::vector<int>(rhs_len + 1, 0));
	for(size_t idx = 0; idx <= lhs_len; ++idx)
	{
		editDist[idx][0] = idx;
	}

	for(size_t idx = 0; idx <= rhs_len; ++idx)
	{
		editDist[0][idx] = idx;
	}

	for(std::size_t dist_i = 1; dist_i <= lhs_len; ++dist_i)
	{
		for(std::size_t dist_j = 1; dist_j <= rhs_len; ++dist_j)
		{
			if(lhsChars[dist_i - 1] == rhsChars[dist_j - 1])
			{
				editDist[dist_i][dist_j] = editDist[dist_i - 1][dist_j - 1];
			}
			else
			{
				editDist[dist_i][dist_j] = triple_min(
					editDist[dist_i][dist_j - 1] + 1,
					editDist[dist_i - 1][dist_j] + 1,
					editDist[dist_i - 1][dist_j - 1] + 1);
			}
		}
	}
	return editDist[lhs_len][rhs_len];
}
