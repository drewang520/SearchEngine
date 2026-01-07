#include "ProtocolParser.h"

void Protocol::ProtocolParser::to_json(json & j, const Message& msg)
{
   j = json{{"length", msg.length},
            {"id", msg.id},
            {"data", msg.data}
   };
}

void Protocol::ProtocolParser::from_json(const json & j, Message& msg)
{
    j.at("length").get_to(msg.length);
    j.at("id").get_to(msg.id);
    j.at("data").get_to(msg.data);
}

string Protocol::ProtocolParser::JsonToString(const json &j)
{
    return j.dump();
}

json Protocol::ProtocolParser::daParse(const string& msg)
{
    return json::parse(msg);
}

json Protocol::ProtocolParser::vecToJson(const vector<string>& recommandWords)
{
    return json(recommandWords);
}

void Protocol::ProtocolParser::jsonToVec(const json &j, vector<string> vec)
{
    vec.clear();
    vec = j.get<vector<string>>();
}

