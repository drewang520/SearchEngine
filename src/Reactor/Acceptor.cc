#include "Acceptor.h"
#include "Logger.h"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

Acceptor::Acceptor(const std::string& ip, unsigned int port)
: m_socket()
, m_addr(ip, port)
{

}

Acceptor::~Acceptor()
{

}

void Acceptor::ready()
{
    setReuseAddress();
    setReusePort();
    bind();
    listen();
}

void Acceptor::bind()
{
    int ret = ::bind(m_socket.getfd(), (struct sockaddr *)m_addr.getInetAddressPtr(), sizeof(struct sockaddr_in));
    if (-1 == ret)
    {
        LOG_ERROR(std::string("bind failed fd=") + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }
}

void Acceptor::listen()
{
    int ret = ::listen(m_socket.getfd(), 128);
    if (-1 == ret)
    {
        LOG_ERROR(std::string("listen failed fd=") + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }
}

int Acceptor::accept()
{
    struct sockaddr_in peerAddr;
    socklen_t socklen = sizeof(peerAddr);
    int netfd = ::accept(m_socket.getfd(), (struct sockaddr *)&peerAddr, &socklen);
    if (-1 == netfd)
    {
        LOG_ERROR(std::string("accept failed fd=") + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }
    return netfd;
}

void Acceptor::setReuseAddress()
{
    int optival = 1;
    int ret = ::setsockopt(m_socket.getfd(), SOL_SOCKET, SO_REUSEADDR, &optival, sizeof(int));
    if (-1 == ret)
    {
        LOG_ERROR(std::string("setsockopt SO_REUSEADDR failed fd=")
                  + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }
}

void Acceptor::setReusePort()
{
    int optival = 1;
    int ret = ::setsockopt(m_socket.getfd(), SOL_SOCKET, SO_REUSEPORT, &optival, sizeof(int));
    if (-1 == ret)
    {
        LOG_ERROR(std::string("setsockopt SO_REUSEPORT failed fd=")
                  + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }
}

int Acceptor::getfd() const
{
    return m_socket.getfd();
}
