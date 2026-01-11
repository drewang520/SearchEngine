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
    WebPage(const string& doc);
    int getDocId() const;

    string getLink() const;
    string getDoc() const;
    string getTitle() const;

private:
    string _doc;
    int _id;
    string _title;
    string _link;
    string _context;
};


#endif
