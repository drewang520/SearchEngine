#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include "Socket.h"
#include "SocketIO.h"
#include "InetAddress.h"
#include <memory>
#include <functional>

class EventLoop;

class TcpConnection
: public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using TcpConnectionCallback = std::function<void (const TcpConnectionPtr& )>; 
public:
    TcpConnection(int fd, EventLoop* loop);
    ~TcpConnection();

    InetAddress getLocalInetAddress() const;
    InetAddress getClientInetAddress() const; 

    void sendMsg(const std::string& msg);
    std::string recvMsg();

    void sendInLoop(const std::string& msg);

    std::string toString();

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
    Socket m_socket;    
    SocketIO m_socketIO;
    InetAddress m_localAddress;
    InetAddress m_clientAddress;

    EventLoop * m_loop;
    TcpConnectionCallback m_connectionCb;
    TcpConnectionCallback m_messageCb;
    TcpConnectionCallback m_closeCb;
    
};


#endif
