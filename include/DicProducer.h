#include "Configuration.h"
/* #include "Jieba.h" */
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <string>

using std::pair;
using std::string;
using std::set;
using std::vector;
using std::map;

class DicProducer
{
public:
    DicProducer(const string&);
    void buildEnDict(const string&);
    void buildCnDict(const string&);
    void createEnIndex();
    void createCnIndex();
    void storeDict(string savefile);
    void storeIndex(string savefile);
    void printFile() const;
    /* void CnDispatch(string sentence); */
private:
    string dealWord(string word);

private:
    vector<string> _files;
    map<string, int> _dict;
    vector<pair<string, int>> _dict2;
    map<string, set<int>> _index;
    // SplitTool * _cuttor;
};

