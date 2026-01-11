#ifndef _PROTOCOL_PARSER_H_
#define _PROTOCOL_PARSER_H_

#include "nlohmann/json.hpp"
#include <string>
#include <vector>

class WebPage;

using json = nlohmann::json;
using std::string;
using std::vector;


namespace Protocol {

struct Message
{
    int id;
    int length;
    string data;
};

class ProtocolParser
{
public:
    static void to_json(json & j, const Message& msg);
    static void from_json(const json & j, Message& msg);
    static json doParse(const string& msg);
    static string JsonToString(const json &j);
    static json vecToJson(const vector<string>& recommandWords);
    static void jsonToVec(const json &j, vector<string>& vec);

    static void to_json(json & j, const WebPage& web);
    static void from_json(const json & j, WebPage& web);
    static json vecWebToJson(const vector<WebPage>& webPage);
    static void jsonToVecWeb(const json &j, vector<WebPage>& vec);
};

};
#endif
