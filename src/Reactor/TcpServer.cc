#include "TcpServer.h"
#include "TcpConnection.h"

TcpServer::TcpServer(const string& ip, unsigned int port)
: _acceptor(ip, port)
, _evtiloop(_acceptor)
{

}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
   _acceptor.ready();

   _evtiloop.loop();
}

void TcpServer::stop()
{

}

void TcpServer::setAllCallback(TcpConnection::TcpConnectionCallback&& connectioncb, 
                               TcpConnection::TcpConnectionCallback&& messagecb,
                               TcpConnection::TcpConnectionCallback&& closecb)
{
    _evtiloop.LoginConnectionCallback(std::move(connectioncb));
    _evtiloop.LoginMessageCallback(std::move(messagecb));
    _evtiloop.LoginCloseCallback(std::move(closecb));
}

