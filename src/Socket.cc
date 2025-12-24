#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>

Socket::Socket()
{
    _socketfd = socket(AF_INET, SOCK_STREAM, 0); 
}

Socket::~Socket()
{
    close(_socketfd);
}

int Socket::getfd() const
{
    return _socketfd;
}

Socket::Socket(int fd)
: _socketfd(fd)
{

}
