#include "TcpConnection.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logger.h"
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>

using std::ostringstream;

TcpConnection::TcpConnection(int fd, EventLoop * loop)
: m_socket(fd)
, m_socketIO(fd)
, m_localAddress(getLocalInetAddress())
, m_clientAddress(getClientInetAddress())
, m_loop(loop)
{

}

TcpConnection::~TcpConnection()
{
   
}

InetAddress TcpConnection::getLocalInetAddress() const
{
    struct sockaddr_in buf_addr{};
    socklen_t addrlen = sizeof(buf_addr);
    int ret = getsockname(m_socket.getfd(), (struct sockaddr *)&buf_addr, &addrlen);
    if (-1 == ret)
    {
        LOG_ERROR(std::string("getsockname failed fd=") + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }
    return InetAddress(buf_addr);
}

InetAddress TcpConnection::getClientInetAddress() const
{
    struct sockaddr_in client_addr{};
    socklen_t addrlen = sizeof(client_addr);
    int ret = getpeername(m_socket.getfd(), (struct sockaddr *)&client_addr, &addrlen);
    
    if (-1 == ret)
    {
        LOG_ERROR(std::string("getpeername failed fd=") + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }
    return InetAddress(client_addr);
}

void TcpConnection::sendMsg(const string& msg)
{
    m_socketIO.writen(msg.c_str(), msg.size());
}

string TcpConnection::recvMsg()
{
    char buf[65535] = {0};
    m_socketIO.readLine(buf, sizeof(buf));
    return string(buf);
}

void TcpConnection::sendInLoop(const string& msg)
{
    if (m_loop)
    {
        auto self = shared_from_this();
        m_loop->runEventLoop([self, msg]() {
            self->sendMsg(msg);
        });
    }
}

string TcpConnection::toString()
{
    ostringstream oss;
    oss << m_clientAddress.getip() << ":"
        << m_clientAddress.getport() << "--->"
        << m_localAddress.getip() << ":"
        << m_localAddress.getport() << " ";

    return string(oss.str()); 
}
int TcpConnection::Getfd() const
{
    return m_socket.getfd();
}

bool TcpConnection::isClosed()
{
    char buf[65535] = {0};
    int ret = ::recv(m_socket.getfd(), buf, sizeof(buf), MSG_PEEK); 
    if (-1 == ret)
    {
        LOG_ERROR(std::string("recv peek failed fd=") + std::to_string(m_socket.getfd())
                  + " reason=" + std::strerror(errno));
    }

    return (0 == ret);
}

void TcpConnection::LoginConnectionCallback(const TcpConnectionCallback & connectioncb)
{
    m_connectionCb = std::move(connectioncb);
}

void TcpConnection::LoginMessageCallback(const TcpConnectionCallback & messagecb)
{
    m_messageCb = std::move(messagecb);
}

void TcpConnection::LoginCloseCallback(const TcpConnectionCallback & closecb)
{
    m_closeCb = std::move(closecb);
}

void TcpConnection::ExConnectionCallback()
{
    if (m_connectionCb)
    {
        m_connectionCb(shared_from_this());
    }
}

void TcpConnection::ExMessageCallback()
{
    if (m_messageCb)
    {
        m_messageCb(shared_from_this());
    }
}

void TcpConnection::ExCloseCallback()
{
    if (m_closeCb)
    {
        m_closeCb(shared_from_this());
    }
}
