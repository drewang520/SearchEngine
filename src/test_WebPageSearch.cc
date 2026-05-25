#include "WebPageSearcher.h"
#include "Configuration.h"
#include <iostream>

int main(int argc, char * argv[])
{
    Configuration& pConfig = Configuration::createpInstance(argc > 1 ? argv[1] : "../config/config.json");
    std::string query = argc > 2 ? argv[2] : "审案";
    WebPageSearch webPageSearch(query, pConfig);
    vector<WebPage> webPage =  webPageSearch.doQuery();
    for (size_t idx = 0; idx < webPage.size() && idx < 10; ++idx)
    {
        std::cout << "id = " << webPage[idx].getDocId() << " " << "_title = " << webPage[idx].getTitle() << "\n";  
    }
    return 0;
}
