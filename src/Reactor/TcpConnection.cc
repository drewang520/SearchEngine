#include "TcpConnection.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>

using std::ostringstream;

TcpConnection::TcpConnection(int fd, EventLoop * loop)
: _socket(fd)
, _socketIO(fd)
, _localAddress(getLocalInetAddress())
, _clientAddress(getClientInetAddress())
, _loop(loop)
{

}

TcpConnection::~TcpConnection()
{
   
}

InetAddress TcpConnection::getLocalInetAddress() const
{
    struct sockaddr_in buf_addr;
    socklen_t addrlen = sizeof(buf_addr);
    int ret = getsockname(_socket.getfd(), (struct sockaddr *)&buf_addr, &addrlen);
    if (-1 == ret)
    {
        perror("getsockname -1");
    }
    return InetAddress(buf_addr);
}

InetAddress TcpConnection::getClientInetAddress() const
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int ret = getpeername(_socket.getfd(), (struct sockaddr *)&client_addr, &addrlen);
    
    if (-1 == ret)
    {
        perror("getpeername -1");
    }
    return InetAddress(client_addr);
}

void TcpConnection::sendMsg(const string& msg)
{
    _socketIO.writen(msg.c_str(), msg.size());
}

string TcpConnection::recvMsg()
{
    char buf[65535] = {0};
    _socketIO.readLine(buf, sizeof(buf));
    
    return string(buf);
}

void TcpConnection::sendInLoop(const string& msg)
{
    if (_loop)
    {
        _loop->runEventLoop(std::bind(&TcpConnection::sendMsg, this, msg));
    }
}

string TcpConnection::toString()
{
    ostringstream oss;
    oss << _clientAddress.getip() << ":"
        << _clientAddress.getport() << "--->"
        << _localAddress.getip() << ":"
        << _localAddress.getport() << " ";

    return string(oss.str()); 
}
int TcpConnection::Getfd() const
{
    return _socket.getfd();
}

bool TcpConnection::isClosed()
{
    char buf[65535] = {0};
    int ret = ::recv(_socket.getfd(), buf, sizeof(buf), MSG_PEEK); 

    return (0 == ret);
}

void TcpConnection::LoginConnectionCallback(const TcpConnectionCallback & connectioncb)
{
    _connectionCb = std::move(connectioncb);
}

void TcpConnection::LoginMessageCallback(const TcpConnectionCallback & messagecb)
{
    _messageCb = std::move(messagecb);
}

void TcpConnection::LoginCloseCallback(const TcpConnectionCallback & closecb)
{
    _closeCb = std::move(closecb);
}

void TcpConnection::ExConnectionCallback()
{
    if (_connectionCb)
    {
        _connectionCb(shared_from_this());
    }
}

void TcpConnection::ExMessageCallback()
{
    if (_connectionCb)
    {
        _messageCb(shared_from_this());
    }
}

void TcpConnection::ExCloseCallback()
{
    if (_closeCb)
    {
        _closeCb(shared_from_this());
    }
}
