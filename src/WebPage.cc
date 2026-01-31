#include "WebPage.h"
#include "tinyxml2.h"

#include <cstdlib>
#include <iostream>

using namespace tinyxml2;

WebPage::WebPage()
: _title()
, _link()
{

}

WebPage::WebPage(const string& doc)
: _doc(doc)
{
    if (!doc.empty())
    {
        XMLDocument xml;
        XMLError error = xml.Parse(doc.c_str());
        if (error == tinyxml2::XML_SUCCESS)
        {
            /* std::cout << "xml parse success" << "\n"; */
            XMLElement * node = xml.RootElement();                        
            string string_id = node->FirstChildElement("docid")->GetText();
            _id = stoi(string_id);
            _title = node->FirstChildElement("title")->GetText();
            _link = node->FirstChildElement("link")->GetText();
            _context = node->FirstChildElement("content")->GetText();
        }
        else 
        {
            std::cout << "xml parse fail" << "\n";
        }
    }
}

int WebPage::getDocId() const
{
    return _id;
}

string WebPage::getTitle() const
{
    return _title;
}

string WebPage::getLink() const
{
    return _link;
}

string WebPage::getDoc() const
{
    return _doc;
}

