
#include <string>
#include <iostream>

using std::cout;
using std::endl;
using std::string;
  
//1. 求取一个字符占据的字节数
size_t nBytesCode(const char ch);

//2. 求取一个字符串的字符长度
std::size_t length(const std::string &str);

//3. 中英文通用的最小编辑距离算法
int editDistance(const std::string & lhs, const std::string &rhs);

void test0()
{
	//std::string本身是一个字节流的字符串
	// 字符流的字符串
	string s1 = "abcd";//4个字符的字符串
	//获取的是字符的长度
	string s2 = "中国";//2个字符的字符串
	for(auto & ch : s2) {
		cout << ch;
	}
	cout << endl;
	cout << "s2[1]: " << s2[1] << endl;
	cout << endl;
	cout << "s1.size() : " << s1.size() << endl;
	cout << "s2.size() : " << s2.size() << endl;

	string s3 = s2.substr(0, 3);
	cout << "s3.size(): " << s3.size() << endl;
	cout << "s3: " << s3 << endl;

	string s4 = s2.substr(1, 3);
	cout << "s4.size(): " << s4.size() << endl;
	cout << "s4: " << s4 << endl;

	//字符的长度
	string s5 = "abc中国";
	cout << "s5.size(): " << s5.size() << endl;	
	cout << "s5.length: " << length(s5) << endl;

	// 中国   => h中国 => a中国 => ab中国 => abc中国
	// s2经过编辑之后，变成s5
	cout << "s2与s5的最小编辑距离: " << editDistance(s2, s5) << endl;

	//dp[0][0]
	//dp[i][j] = 
	//A: 空串
	//B: abc
}

int main(void)
{
	test0();
	return 0;
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

int editDistance(const std::string & lhs, const std::string &rhs)
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

