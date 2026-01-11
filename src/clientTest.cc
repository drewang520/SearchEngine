#include "Configuration.h"
#include "ProtocolParser.h"
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
     system("google-chrome file:///home/komorebi/study/project/search_engine/data/html/client.html");
}

void test() 
{
    Configuration * config = Configuration::createpInstance();

	int clientfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if(clientfd < 0)
    {
		perror("socket");
		return ;
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(stoi(config->getConfig().at("port")));
    serverAddr.sin_addr.s_addr = inet_addr(config->getConfig().at("ip").c_str());
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
		cout<< ">> pls input some message:";
		getline(cin, line);
        msg.id = 1;
        msg.data = line;
        msg.length = line.size();
        ProtocolParser::to_json(j, msg);
        string message = ProtocolParser::JsonToString(j);
        message.append("\n");
		//1. 客户端先发数据
		send(clientfd, message.data(), message.size(), 0);
		/* close(clientfd); */

		char buff[4096] = {0};
		recv(clientfd, buff, sizeof(buff), 0);
		printf("recv msg from server: %s\n", buff);

        string KeyWords(buff);
		cout << ">> pls choose one KeyWord:";
		getline(cin, line);
        vector<string> vec;
        ProtocolParser::jsonToVec(ProtocolParser::doParse(KeyWords), vec); 
        msg.id = 2;
        msg.data = line;
        msg.length = line.size();

        ProtocolParser::to_json(j, msg);
        message = ProtocolParser::JsonToString(j);
        message.append("\n");
		send(clientfd, message.data(), message.size(), 0);

        bzero(buff, sizeof(buff));
		recv(clientfd, buff, sizeof(buff), 0);
		printf("recv msg from server: %s\n", buff);
        json j = ProtocolParser::doParse(buff);        
        if (!j.is_array())
        {
            std::cout << "not found" << "\n";
            continue;
        }
        LoadHtml(j);
	}
	close(clientfd);
} 
 
int main(void)
{
	test();
	return 0;
}
