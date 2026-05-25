#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>

Socket::Socket()
{
    m_socketfd = socket(AF_INET, SOCK_STREAM, 0); 
}

Socket::~Socket()
{
    close(m_socketfd);
}

int Socket::getfd() const
{
    return m_socketfd;
}

Socket::Socket(int fd)
: m_socketfd(fd)
{

}
