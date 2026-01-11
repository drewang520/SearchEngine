#include "KeyRecommander.h"

KeyRecommander::KeyRecommander(string queryWords, const Configuration * config)
: _queryWords(queryWords)
, _pDict(new Dictionary(config))
, _prique()
, _config(config)
{

}

vector<string> KeyRecommander::doQuery()
{
    map<string, set<int>> index = _pDict->getIndex();
    vector<pair<string, int>> dict = _pDict->getDict();
    vector<string> queryResult;
    /* queryResult.reserve(20); */
    /* set<CandidateResult, CandidateResultCompare> candidataWords; */
    map<string, int> candidataWords;
    for (size_t i = 0; i < _queryWords.size(); )
    {
        if ((_queryWords[i] & 0x80) == 0)
        {
            string singlie_ch = _queryWords.substr(i, 1);
            queryIndex(candidataWords, index, dict, singlie_ch);
            ++i;
        }
        else
        {
            int Cn_length = 0;
            unsigned char c = _queryWords[i];
            if (c >= 0xF0) Cn_length = 4;
            else if (c >= 0xE0) Cn_length = 3;
            else if (c >= 0xC0) Cn_length = 2;
            string singlie_ch = _queryWords.substr(i, Cn_length);
            queryIndex(candidataWords, index, dict, singlie_ch);
            i += Cn_length;
        }
    }
    return candidataSort(candidataWords, queryResult);
}

void KeyRecommander::queryIndex(map<string, int>& candidataWords, map<string, set<int>>& index, 
                                    vector<pair<string, int>>& dict, const string& single_ch)
{
    map<string, set<int>>::iterator it = index.find(single_ch);
    if (it != index.end())
    {
        for (const auto &elem : it->second)
        {
            /* candidataWords.insert({dict[elem].first, dict[elem].second}); */
            candidataWords[dict[elem].first] = dict[elem].second;
        }
    }
}

vector<string> KeyRecommander::candidataSort(map<string, int>& candidataWords, vector<string>& queryResult)
{
    CandidateResult canresult;
    for (const auto& [word, frequency] : candidataWords)
    {
        canresult._word = word;
        canresult._freq = frequency;
        canresult._dist = editDistance(canresult._word, _queryWords);
        _prique.push(canresult);         
    }

    size_t candidataNum = 0;
    while (!_prique.empty() && candidataNum != stoi(_config->getConfig().at("candidataNum")))
    {
        queryResult.push_back(_prique.top()._word);
        _prique.pop();
        ++candidataNum;
    }

    return queryResult;
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

int KeyRecommander::editDistance(const std::string & lhs, const std::string &rhs)
{//计算最小编辑距离-包括处理中英文
	size_t lhs_len = length(lhs);
	size_t rhs_len = length(rhs);
	int editDist[lhs_len + 1][rhs_len + 1];
	for(size_t idx = 0; idx <= lhs_len; ++idx)
	{
		editDist[idx][0] = idx;
	}

	for(size_t idx = 0; idx <= rhs_len; ++idx)
	{
		editDist[0][idx] = idx;
	}

	std::string sublhs, subrhs;
	for(std::size_t dist_i = 1, lhs_idx = 0; dist_i <= lhs_len; ++dist_i, ++lhs_idx)
	{
		size_t nBytes = nBytesCode(lhs[lhs_idx]);
		sublhs = lhs.substr(lhs_idx, nBytes);
		lhs_idx += (nBytes - 1);

		for(std::size_t dist_j = 1, rhs_idx = 0; dist_j <= rhs_len; ++dist_j, ++rhs_idx)
		{
			nBytes = nBytesCode(rhs[rhs_idx]);
			subrhs = rhs.substr(rhs_idx, nBytes);
			rhs_idx += (nBytes - 1);
			if(sublhs == subrhs)
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

