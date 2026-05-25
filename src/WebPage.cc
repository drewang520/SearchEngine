#include "WebPage.h"
#include "tinyxml2.h"

#include <cstdlib>
#include <iostream>

using namespace tinyxml2;

WebPage::WebPage()
: m_title()
, m_link()
{

}

WebPage::WebPage(const string& doc)
: m_doc(doc)
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
            m_id = stoi(string_id);
            m_title = node->FirstChildElement("title")->GetText();
            m_link = node->FirstChildElement("link")->GetText();
            m_context = node->FirstChildElement("content")->GetText();
        }
        else 
        {
            std::cout << "xml parse fail" << "\n";
        }
    }
}

int WebPage::getDocId() const
{
    return m_id;
}

string WebPage::getTitle() const
{
    return m_title;
}

string WebPage::getLink() const
{
    return m_link;
}

string WebPage::getDoc() const
{
    return m_doc;
}

