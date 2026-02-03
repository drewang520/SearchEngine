#include "crawl.h"
#include "pybind11/embed.h"
#include "tinyxml2.h"
#include <string>
#include <iostream>

using namespace pybind11;

Crawl_XML::Crawl_XML(const std::string& xmllink)
: _link(xmllink)
{
    std::string xml_content;
    pybind11::scoped_interpreter guard{}; //初始化 Python解释器
    
    pybind11::module_ sys = pybind11::module_::import("sys");
    std::string moudule_dir = "/home/drewang/study/Python/";
    sys.attr("path").attr("append")(moudule_dir);
    try {
        pybind11::module_ cale = pybind11::module_::import("obtain_xml");
        pybind11::object raw_xml = cale.attr("download_xml")(_link);
        xml_content = raw_xml.cast<std::string>();
        if (xml_content.find("Error:") == 0)
        {
            std::cerr << "爬取失败: " << xml_content << "\n";
        }
    } catch (pybind11::error_already_set &e){
        std::cerr << "Python error: " << e.what() << "\n";
    }

    /* std::cout << "xml content: " << xml_content << "\n"; */
    doPrase(xml_content);
}

void Crawl_XML::doPrase(const std::string& xml_content)
{
    XMLDocument xml;
    xml.Parse(xml_content.c_str());
    if (xml.ErrorID())
    {
        std::cout << "LoadFile fail" << "\n";
        return;
    }
    XMLElement * node = xml.FirstChildElement("sitemapIndex")->FirstChildElement("sitemap");
    while (node)
    {
        std::string field = node->FirstChildElement("loc")->GetText();
        /* std::cout << "field = " << field << "\n"; */
        splitLink(field);
        node = node->NextSiblingElement("sitemap");
    }
}

void Crawl_XML::splitLink(const std::string& link)
{
    size_t last_slash = link.find_last_of('/');
    /* std::string sub = link.substr(0, last_slash); */
    if (last_slash != std::string::npos && last_slash > 0)
    {
        size_t second_lash_slash = link.find_last_of('/', last_slash - 1);
        std::string field= link.substr(second_lash_slash + 1, last_slash - second_lash_slash - 1);
        _fields.push_back(field);
        /* std::cout << "提取的类别是：" << field << "\n"; */
    }
}

void Crawl_XML::print()
{
    for (auto & field : _fields)
    {
        std::cout << field << "\n";
    }
}

