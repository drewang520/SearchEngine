#include "Configuration.h"
#include "ProtocolParser.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

using namespace Protocol;
using std::cin;
using std::cout;
using std::string;
 
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
    serverAddr.sin_port = htons(stoi(config->getConfig()["port"]));
    serverAddr.sin_addr.s_addr = inet_addr(config->getConfig()["ip"].c_str());
	socklen_t length = sizeof(serverAddr);

	if(::connect(clientfd,(struct sockaddr*)&serverAddr, length) < 0)
    {
		perror("connect");
		close(clientfd);
		return;
	}
	printf("conn has connected!...\n");

    Message msg;
    msg.id = 1;
    json j; 
	while(1) 
    {
		string line;
		cout << ">> pls input some message:";
		getline(cin, line);
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
        ProtocolParser::jsonToVec(ProtocolParser::daParse(KeyWords), vec); 
        msg.id = 2;
        msg.data = line;
        msg.length = line.size();

        ProtocolParser::to_json(j, msg);
        message = ProtocolParser::JsonToString(j);
        message.append("\n");
		send(clientfd, message.data(), message.size(), 0);

		recv(clientfd, buff, sizeof(buff), 0);
		printf("recv msg from server: %s\n", buff);
	}

	close(clientfd);
} 
 
int main(void)
{
	test();
	return 0;
}
