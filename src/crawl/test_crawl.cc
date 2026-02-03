#include "crawl.h"

int main(int argc, char * argv[])
{
    Crawl_XML xmlcrawl("http://www.people.cn/sitemap_index.xml");
    xmlcrawl.print();
    return 0;
}

