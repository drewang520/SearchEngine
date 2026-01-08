#include "ProtocolParser.h"
#include "WebPage.h"

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

void Protocol::ProtocolParser::jsonToVec(const json &j, vector<string>& vec)
{
    vec.clear();
    vec = j.get<vector<string>>();
}

void to_json(json & j, const WebPage& web)
{
    j = json{{"title", web._title},
                 {"link", web._link}
    };
}

void from_json(const json & j, WebPage& web)
{
    j.at("title").get_to(web._title);
    j.at("link").get_to(web._link);
}

json Protocol::ProtocolParser::vecWebToJson(const vector<WebPage>& web)
{
    if (web.empty())
    {
        return json("no found");
    }
    return json(web);
}

void Protocol::ProtocolParser::jsonToVecWeb(const json &j, vector<WebPage>& vec)
{
    vec.clear();
    /* vec = j.get<vector<WebPage>>(); */
}

