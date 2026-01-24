#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include "Socket.h"
#include "SocketIO.h"
#include "InetAddress.h"
#include <memory>
#include <functional>

using std::function;
using std::shared_ptr;

class EventLoop;

class TcpConnection
: public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = shared_ptr<TcpConnection>;
    using TcpConnectionCallback = function<void (const TcpConnectionPtr& )>; 
public:
    TcpConnection(int fd, EventLoop* loop);
    ~TcpConnection();

    InetAddress getLocalInetAddress() const;
    InetAddress getClientInetAddress() const; 

    void sendMsg(const string& msg);
    string recvMsg();

    void sendInLoop(const string& msg);

    string toString();

    int Getfd() const;

    bool isClosed();

    void LoginConnectionCallback(const TcpConnectionCallback &connectioncb);
    void LoginMessageCallback(const TcpConnectionCallback & messagecb);
    void LoginCloseCallback(const TcpConnectionCallback & closecb);

    void ExConnectionCallback();
    void ExMessageCallback();
    void ExCloseCallback();

private:
    // 注意数据成员书写的顺序与初始化列表的顺序
    // 可能会出现前后依赖而出错
    // 保持初始化列表顺序与声明顺序一致
    Socket _socket;    
    SocketIO _socketIO;
    InetAddress _localAddress;
    InetAddress _clientAddress;

    EventLoop * _loop;
    TcpConnectionCallback _connectionCb;
    TcpConnectionCallback _messageCb;
    TcpConnectionCallback _closeCb;
    
};


#endif
