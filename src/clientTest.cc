#include "Configuration.h"
#include "ProtocolParser.h"
#include "WebPage.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace Protocol;
using std::cin;
using std::cout;
using std::string;

string RecvString(int fd)
{
    vector<char> buffer(65536);
    ssize_t nread = recv(fd, buffer.data(), buffer.size(), 0);
    if (nread <= 0)
    {
        return {};
    }
    return string(buffer.data(), static_cast<size_t>(nread));
}

void SendMessage(int fd, Message& msg)
{
    json j;
    ProtocolParser::to_json(j, msg);
    string message = ProtocolParser::JsonToString(j);
    message.append("\n");
    send(fd, message.data(), message.size(), 0);
}

void PrintAIAnswer(const json& j)
{
    std::cout << "\nAI answer:\n" << j.value("answer", "") << "\n";

    if (j.contains("sources") && j["sources"].is_array())
    {
        std::cout << "\nSources:\n";
        for (const auto& item : j["sources"])
        {
            std::cout << "[" << item.value("index", 0) << "] "
                      << item.value("title", "") << "\n"
                      << item.value("link", "") << "\n";
        }
    }

    if (j.contains("related_queries") && j["related_queries"].is_array())
    {
        std::cout << "\nRelated queries:\n";
        for (const auto& item : j["related_queries"])
        {
            std::cout << "- " << item.get<string>() << "\n";
        }
    }
    std::cout << "\n";
}
 
void LoadHtml(json & j)
{
     std::ofstream ofs("../data/html/client.html");
     ofs << "<!DOCTYPE html>\n";
     ofs << "<html lang='zh-CN'>\n";
     ofs << "<head>\n";
     ofs << "<meta charset='UTF-8'>\n";
     ofs << "<title>搜索结果</title>\n";
     ofs << "<style>\n"
           "body { font-family: Arial, sans-serif; }\n"
           ".item { margin-bottom: 20px; }\n"
           ".title { font-size: 18px; font-weight: bold; }\n"
           ".link { color: #1a0dab; text-decoration: none; }\n"
           "</style>\n";
     ofs << "</head>\n";
     ofs << "<body>\n";
     ofs << "<h1>搜索结果</h1>\n";
     for (size_t idx = 0; idx < j.size(); ++idx)
     {
        ofs << "<div class='item'>\n";
        ofs << "<div class='title'>" << j[idx]["title"] << "</div>\n";
        ofs << "<a class='link' href=" << j[idx]["link"] << ">" << j[idx]["link"] << "</a>\n";
        ofs << "</div\n>";
     }
     ofs << "</body></html>\n";
     ofs.close();
     system("google-chrome file:///home/drewang/study/project/search_engine/data/html/client.html");
}

void test(const std::string& configPath) 
{
    Configuration& config = Configuration::createpInstance(configPath);

	int clientfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if(clientfd < 0)
    {
		perror("socket");
		return ;
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(stoi(config.getConfig().at("port")));
    serverAddr.sin_addr.s_addr = inet_addr(config.getConfig().at("ip").c_str());
	socklen_t length = sizeof(serverAddr);

	if(::connect(clientfd,(struct sockaddr*)&serverAddr, length) < 0)
    {
		perror("connect");
		close(clientfd);
		return;
	}
	printf("conn has connected!...\n");

    Message msg;
    json j;
	while(1) 
    {
		string line;
		cout<< ">> pls input some message, or use /ai question:";
        if (!getline(cin, line))
        {
            break;
        }

        if (line.rfind("/ai ", 0) == 0)
        {
            msg.id = Protocol::AI_SEARCH;
            msg.data = line.substr(4);
            msg.length = msg.data.size();
            SendMessage(clientfd, msg);

            string response = RecvString(clientfd);
            if (response.empty())
            {
                std::cout << "server closed connection\n";
                break;
            }
            PrintAIAnswer(ProtocolParser::doParse(response));
            continue;
        }

        msg.id = Protocol::KEY_RECOMMAND;
        msg.data = line;
        msg.length = line.size();
        SendMessage(clientfd, msg);

        string word = RecvString(clientfd);
        if (word.empty())
        {
            std::cout << "server closed connection\n";
            break;
        }
        j = ProtocolParser::doParse(word);
        vector<string> words;
        ProtocolParser::jsonToVec(j, words);
        std::cout << "recv word from server: " << "\n";
        for (auto &elem :words)
        {
            std::cout << elem << "\n";
        }

        /* string KeyWords(buff); */
		cout << ">> pls choose one KeyWord:";
		getline(cin, line);
        /* vector<string> vec; */
        /* ProtocolParser::jsonToVec(ProtocolParser::doParse(KeyWords), vec); */ 
        msg.id = Protocol::WEBPAGE_SEARCH;
        msg.data = line;
        msg.length = line.size();

        SendMessage(clientfd, msg);
        vector<WebPage> webWords;
        /* bzero(buff, sizeof(buff)); */
        string Web = RecvString(clientfd);
        if (Web.empty())
        {
            std::cout << "server closed connection\n";
            break;
        }
        j = ProtocolParser::doParse(Web);        
        /* std::cout << "recv web from server: " << "\n"; */
        if (!j.is_array())
        {
            /* std::cout << "not found" << "\n"; */
            system("google-chrome file:///home/drewang/study/project/search_engine/data/html/error.html");
            continue;
        }
        ProtocolParser::jsonToVecWeb(j, webWords);
        std::cout << "webWords.size()" << webWords.size() << "\n";
        for (auto &elem :webWords)
        {
            std::cout << "Web: \n" << "title: " << elem.getTitle() << "\n"
                << "link: " << elem.getLink() << "\n"
                << "\n";
        }
        LoadHtml(j);
	}
	close(clientfd);
} 
 
int main(int argc, char * argv[])
{
	test(argc > 1 ? argv[1] : "../config/config.json");
	return 0;
}
