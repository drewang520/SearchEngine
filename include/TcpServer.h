#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "Acceptor.h"
#include "EventLoop.h"
#include "TcpConnection.h"

#include <string>

using std::string;
class TcpServer
{
public:
    TcpServer(const string& ip, unsigned int port);
    ~TcpServer();
    
    void start();
    void stop();

    void setAllCallback(TcpConnection::TcpConnectionCallback && connectioncb, 
                        TcpConnection::TcpConnectionCallback && messagecb, 
                        TcpConnection::TcpConnectionCallback && closecb);


private:
    Acceptor _acceptor;
    EventLoop _evtiloop;
};

#endif
