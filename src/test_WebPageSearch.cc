#include "WebPageSearcher.h"
#include "Configuration.h"
#include <string>

int main()
{
    Configuration::setConfigurFilePath("../config/config.json");
    Configuration * pConfig = Configuration::createpInstance();

    std::map<std::string, string> _config = pConfig->getConfig();

    WebPageQuery webQuery(pConfig->getConfig()["invertIndexTable"], pConfig->getConfig()["newoffset"]);
    webQuery.doQuery("雷军是小米集团的董事长");

}
