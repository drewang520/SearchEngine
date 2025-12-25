#include "Acceptor.h"
#include <asm-generic/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>

Acceptor::Acceptor(const string& ip, unsigned int port)
: _socket()
, _addr(ip, port)
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
    int ret = ::bind(_socket.getfd(), (struct sockaddr *)_addr.getInetAddressPtr(), sizeof(struct sockaddr_in));
    if (-1 == ret)
    {
        perror("bind -1");
    }
}

void Acceptor::listen()
{
    int ret = ::listen(_socket.getfd(), 128);
    if (-1 == ret)
    {
        perror("listen -1");
    }
}

int Acceptor::accept()
{
    socklen_t socklen = sizeof(struct sockaddr_in);
    int netfd = ::accept(_socket.getfd(), (struct sockaddr *)_addr.getInetAddressPtr(), &socklen);
    return netfd;
}

void Acceptor::setReuseAddress()
{
    int optival = 1;
    int ret = ::setsockopt(_socket.getfd(), SOL_SOCKET, SO_REUSEADDR, &optival, sizeof(int));
    if (-1 == ret)
    {
        perror("setsockopt -1");
    }
}

void Acceptor::setReusePort()
{
    int optival = 1;
    int ret = ::setsockopt(_socket.getfd(), SOL_SOCKET, SO_REUSEPORT, &optival, sizeof(int));
    if (-1 == ret)
    {
        perror("setsockopt -1");
    }
}

int Acceptor::getfd() const
{
    return _socket.getfd();
}
