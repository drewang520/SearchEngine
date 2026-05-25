#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "Acceptor.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include <string>

class TcpServer
{
public:
    TcpServer(const std::string& ip, unsigned int port);
    ~TcpServer();
    
    void start();
    void stop();

    void setAllCallback(TcpConnection::TcpConnectionCallback && connectioncb, 
                        TcpConnection::TcpConnectionCallback && messagecb, 
                        TcpConnection::TcpConnectionCallback && closecb);


private:
    Acceptor m_acceptor;
    EventLoop m_eventLoop;
};

#endif
