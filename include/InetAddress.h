#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

class InetAddress
{
public:
    InetAddress(const std::string& ip, unsigned int port);
    InetAddress(const struct sockaddr_in& addr);
    ~InetAddress();

    std::string getip() const;
    unsigned int getport() const;
    const struct sockaddr_in * getInetAddressPtr() const;

private:
    struct sockaddr_in m_addr;
};

#endif
