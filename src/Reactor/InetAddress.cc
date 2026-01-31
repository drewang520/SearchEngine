#include "InetAddress.h"
#include <netinet/in.h>
#include <sys/socket.h>


InetAddress::InetAddress(const string& ip, unsigned int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    _addr.sin_addr.s_addr = inet_addr(ip.c_str()); 
}

InetAddress::~InetAddress()
{

}

string InetAddress::getip() const
{
    return inet_ntoa(_addr.sin_addr);
}

unsigned int InetAddress::getport() const
{
    return ntohs(_addr.sin_port);
}

const struct sockaddr_in * InetAddress::getInetAddressPtr() const
{
    return &_addr;
}

InetAddress::InetAddress(const struct sockaddr_in& addr)
: _addr(addr)
{

}
