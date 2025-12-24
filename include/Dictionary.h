#include <utility>
#include <vector>
#include <set>
#include <map>
#include <string>

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::set;

class Dictionary
{
public:
    Dictionary();
    vector<pair<string, int>> & getDict();
    map<string, set<int>> & getIndex();

private:
    void init();
    void queryindex();
    int distance(string candidate);
private:
    vector<pair<string, int>> _dict;
    map<string, set<int>> _index;
};
