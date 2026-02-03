#include <tinyxml2.h>
#include <string>
#include <vector>

using namespace tinyxml2;

class Crawl_XML
{
public:
    Crawl_XML(const std::string&);
    void doPrase(const std::string&);
    void print();
private:
    void splitLink(const std::string&);

private:
    std::string _link;
    std::vector<std::string> _fields; 
};
