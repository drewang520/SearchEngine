#include "InetAddress.h"
#include <netinet/in.h>
#include <sys/socket.h>


InetAddress::InetAddress(const std::string& ip, unsigned int port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str()); 
}

InetAddress::~InetAddress()
{

}

std::string InetAddress::getip() const
{
    return inet_ntoa(m_addr.sin_addr);
}

unsigned int InetAddress::getport() const
{
    return ntohs(m_addr.sin_port);
}

const struct sockaddr_in * InetAddress::getInetAddressPtr() const
{
    return &m_addr;
}

InetAddress::InetAddress(const struct sockaddr_in& addr)
: m_addr(addr)
{

}
