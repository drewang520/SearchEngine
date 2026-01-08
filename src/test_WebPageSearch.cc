#include "WebPageSearcher.h"
#include "Configuration.h"
#include <iostream>

int main()
{
    Configuration * pConfig = Configuration::createpInstance();
    WebPageSearch webPageSearch("hello", pConfig);
    vector<WebPage> webPage =  webPageSearch.doQuery();
    for (size_t idx = 0; idx < webPage.size() && idx < 10; ++idx)
    {
        std::cout << "id = " << webPage[idx].getDocId() << " " << "_title = " << webPage[idx].getTitle() << "\n";  
    }
    return 0;
}
