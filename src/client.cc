#include "Configuration.h"
#include "TLVMessage.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

int main(int argc, char *argv[])
{
    Configuration::setConfigurFilePath("../config/config.json");
    Configuration * config = Configuration::createpInstance();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in _addr;
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(stoi(config->getConfig()["port"]));
    _addr.sin_addr.s_addr = inet_addr(config->getConfig()["ip"].c_str());

    int ret = connect(sockfd, (struct sockaddr *)&_addr, sizeof(_addr));
    char * buf = new char[4096]();
    char * netbuf = new char[4096]();
    char * callback = new char[4096]();
    while (1)
    {
        bzero(buf, 4096);
        scanf("%s\n", buf);
        TLV::TLVMessage message(0x01, strlen(buf), buf);
        size_t message_length = 1 + 2 + strlen(buf);
        message.encodeMessage(netbuf);
        send(sockfd, netbuf, message_length, 0);
        int ret = recv(sockfd, callback, sizeof(buf), 0);
        while (ret != 0)
        {
            std::cout << callback<< "\n";
        }
    }
    delete [] buf;
    delete [] netbuf;
    delete [] callback;

    return 0;
}
