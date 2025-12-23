#include "cppjieba/Jieba.hpp"
#include <vector>
#include <string>

using std::string;
using std::vector;

class Jieba
{
public:
    Jieba()
    {
    }

    void CnDispatch(string line, vector<string> files)
    {
        _words.clear();
        const char * dict_path = "../raw_data/module1/dict/jieba.dict.utf8";
        const char * model_path = "../raw_data/module1/dict/hmm_model.utf8";
        const char * user_dict_path = "../raw_data/module1/dict/user.dict.utf8";
        const char * idf_path = "../raw_data/module1/dict/idf.utf8";
        const char * stop_word_path = "../raw_data/module1/dict/stop_words.utf8";
        cppjieba::Jieba jieba(dict_path, model_path, user_dict_path, idf_path, stop_word_path);
        jieba.Cut(line, _words, true);
    
        if (!_words.empty())
        {
             for (size_t i = 0; i < _words.size(); ++i)
             {
                 files.push_back(_words[i]);
             }
        }
    }
private:
    vector<string> _words;
};


