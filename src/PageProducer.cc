#include "PageProducer.h"
#include "simhash/Simhasher.hpp"
#include "tinyxml2.h"
#include <cstddef>
#include <cstdint>
#include <dirent.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <regex>

using namespace tinyxml2;
using namespace simhash;

PageProducer::PageProducer(const Configuration * config)
: _config(config)
{
    const string pagePath = _config->getConfig().at("page_src");
    DIR * pdir = opendir(pagePath.c_str());
    if (pdir != nullptr)
    {
        struct dirent * pdirent;        
        while ((pdirent = readdir(pdir)) != nullptr)
        {
            std::cout << "filename: " << pdirent->d_name << "\n";
            string filepath = pagePath + '/' + pdirent->d_name;
            create(filepath, pdirent->d_name);  
        }
    }
}

void PageProducer::create(const string& filepath, const string& filename)
{
    tinyxml2::XMLDocument xml;
    xml.LoadFile(filepath.c_str());
    if (xml.ErrorID())
    {
        std::cerr << "LoadFile fail" << "\n";
        return;
    }

    XMLElement * itemNode = xml.FirstChildElement("rss")->FirstChildElement("channel")
                                ->FirstChildElement("item");
    while (itemNode)
    {
        string title = itemNode->FirstChildElement("title")->GetText();
        string link = itemNode->FirstChildElement("link")->GetText();
        string description = itemNode->FirstChildElement("description")->GetText();

        if (filename == "coolshell.xml")
        {
            description = itemNode->FirstChildElement("content:encoded")->GetText();
        }
        else if (filename == "dataparse.xml" || filename == "latest.xml")
        {
            description = itemNode->FirstChildElement("content")->GetText();
        }

        std::regex reg("( |　|&nbsp;|[\r]|[\n]|<[^>]+>|showPlayer[(]+[^)]+[)];)"); //通用正则表达式
        description = std::regex_replace(description, reg, "");
        title = std::regex_replace(title, reg, "");

        RSSItem rssitem;
        rssitem._title = title;
        rssitem._link = link;
        rssitem._description = description;

        if (!rssitem._description.empty())
        {
            _page.push_back(rssitem);
        }

        itemNode = itemNode->NextSiblingElement("item");
    }
}

void PageProducer::store()
{
    std::ofstream ofs(_config->getConfig().at("ripepage"));        
    std::ofstream ofs2(_config->getConfig().at("pageoffset"));
    std::ofstream::pos_type p1, p2;
    for (size_t idx = 0; idx < _page.size(); ++idx)
    {
        p1 = ofs.tellp();
        ofs << "<doc>\n" 
            << "\t<docid>"  << idx + 1 << "</docid>\n" 
            << "\t<title>" << _page[idx]._title << "</title>\n"
            << "\t<link>" << _page[idx]._link << "</link>\n"
            << "\t<content>" << _page[idx]._description << "</content>\n"
            << "</doc>\n";
        p2 = ofs.tellp();
        ofs2 << idx + 1 << " " << std::to_string(p1) << " " << std::to_string(p2 - p1) << "\n";
    }
    ofs.close();
    ofs2.close();
}

void PageProducer::pageDeduplicat()
{
    /* vector<uint64_t> simhash; */
    unordered_map<int, uint64_t> simhash;
    const char * dicpath = _config->getConfig().at("dict_path").c_str();
    const char * modelpath = _config->getConfig().at("model_path").c_str();
    const char * idfpath = _config->getConfig().at("idf_path").c_str();
    const char * stopwords = _config->getConfig().at("stop_word_path").c_str();

    Simhasher simhasher(dicpath, modelpath, idfpath, stopwords);    
    uint64_t value;
    size_t topN = 50;
    bool isdedup = false;
    for (size_t idx = 0; idx < _page.size(); ++idx)
    {
        isdedup = false;
        simhasher.make(_page[idx]._description, topN, value);
        for (auto & [pageid, hashvalue] : simhash)
        {
            if (Simhasher::isEqual(hashvalue, value, 3))
            {
                isdedup = true;
                break; 
            }
        }
        if (! isdedup)
        {
            simhash[idx] = value;
        }
    }
    // simhash时间复杂度 O(N * N)

    std::ofstream ofs(_config->getConfig().at("newripepage"));
    std::ofstream ofs2(_config->getConfig().at("newoffset"));
    size_t number = 0;
    std::fstream::pos_type p1, p2;
    /* int prepos = 0, pos = 0, length = 0; */
    /* pair<int, int> pos_len; */
    for (auto & [pageid, hashvalue] : simhash)
    {
        p1 = ofs.tellp();
        ofs << "<doc>\n" 
            << "\t<docid>"  << number + 1 << "</docid>\n" 
            << "\t<title>" << _page[pageid]._title << "</title>\n"
            << "\t<link>" << _page[pageid]._link << "</link>\n"
            << "\t<content>" << _page[pageid]._description << "</content>\n"
            << "</doc>\n";
        p2 = ofs.tellp();
        ofs2 << number + 1 << " " << std::to_string(p1) << " " << std::to_string(p2 - p1) << "\n";
        ++number;
        /* pos = ofs.tellp(); */
        /* length = pos - prepos; */
        /* pos_len = {prepos, length}; */
        /* _offsetPage.insert({(number + 1), pos_len}); */
        /* prepos = pos + 1; */
        /* ++number; */
    }
    ofs.close();
    ofs2.close();
}



