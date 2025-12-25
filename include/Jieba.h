#include "cppjieba/Jieba.hpp"
#include "NoCopyable.h"
#include <vector>
#include <string>

using std::string;
using std::vector;

class Jieba
: public NoCopyable
{
public:
    Jieba()
    {
        const char * dict_path = "../raw_data/module1/dict/jieba.dict.utf8";
        const char * model_path = "../raw_data/module1/dict/hmm_model.utf8";
        const char * user_dict_path = "../raw_data/module1/dict/user.dict.utf8";
        const char * idf_path = "../raw_data/module1/dict/idf.utf8";
        const char * stop_word_path = "../raw_data/module1/dict/stop_words.utf8";
        cppjieba::Jieba _jieba(dict_path, model_path, user_dict_path, idf_path, stop_word_path);
    }

    void Cut(const string& sentence, vector<string>& words)
    {
        _jieba.Cut(sentence, words, true);
    }

private:
    cppjieba::Jieba _jieba;
};


