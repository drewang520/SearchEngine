#ifndef _PAGE_PRODUCER_H
#define _PAGE_PRODUCER_H

#include "Configuration.h"
#include <vector>
#include <string>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::pair;

struct RSSItem
{
    string m_title;
    string m_link;
    string m_description;
};

class PageProducer
{
public:
    PageProducer(const Configuration& config);
    void store();
    void pageDeduplicat();

private:
    void create(const string& filepath, const string& filename);

private:
    vector<RSSItem> m_page;
    map<int, pair<int, int>> m_offsetPage;
    const Configuration& m_config;
};

#endif
