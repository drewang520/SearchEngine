#include "../include/simhash/Simhasher.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using std::string;
using std::cout;
using std::ifstream;
using std::istringstream;

using namespace simhash;

void test1()
{
    ifstream ifs("hello.txt");
    /* string line; */
    /* while (std::getline(ifs, line)) */
    /* { */
        /* istringstream isf(line); */
        string doc, title, link, content, docid;
        while ( ifs >> doc >> docid >> title >> link >> content)
        {
            std::cout << "doc: " << doc << "\n" 
                << "docid = " << docid << "\n"
                << "title: " << title << "\n" 
                << "link: " << link << "\n"
                << "content" << content << "\n";
            std::cout << "\n";
        }
    /* } */

}

void test2()
{
    ifstream ifs("hello.txt");
    ifs.seekg(0);
    char * page = new char[3166];
    ifs.read(page, 3166 - 14);
    string onepage(page);
    istringstream iss(onepage);
    string doc, title, link, content, docid;
    while (iss >> doc >> docid >> title >> link >> content)
    {
        std::cout << "doc: " << doc << "\n" 
            << "docid = " << docid << "\n"
            << "title: " << title << "\n" 
            << "link: " << link << "\n"
            << "content" << content << "\n";
    }
}

int main(int argc, char * argv[])
{
    test1();
}

