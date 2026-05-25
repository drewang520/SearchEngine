#ifndef _WEBPAGE_H
#define _WEBPAGE_H

#include "Configuration.h"
#include "ProtocolParser.h"
#include <string>

using std::string;

class WebPage
{
    friend void Protocol::ProtocolParser::to_json(json & j, const WebPage& web);
    friend void Protocol::ProtocolParser::from_json(const json & j, WebPage& web);

public:
    WebPage();
    WebPage(const string& doc);
    int getDocId() const;

    string getLink() const;
    string getDoc() const;
    string getTitle() const;
    string getContent() const;

private:
    string m_doc;
    int m_id;
    string m_title;
    string m_link;
    string m_context;
};


#endif
