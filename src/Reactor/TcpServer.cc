#include "TcpServer.h"
#include "TcpConnection.h"

TcpServer::TcpServer(const std::string& ip, unsigned int port)
: m_acceptor(ip, port)
, m_eventLoop(m_acceptor)
{

}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
   m_acceptor.ready();
   m_eventLoop.loop();
}

void TcpServer::stop()
{

}

void TcpServer::setAllCallback(TcpConnection::TcpConnectionCallback&& connectioncb, 
                               TcpConnection::TcpConnectionCallback&& messagecb,
                               TcpConnection::TcpConnectionCallback&& closecb)
{
    m_eventLoop.LoginConnectionCallback(std::move(connectioncb));
    m_eventLoop.LoginMessageCallback(std::move(messagecb));
    m_eventLoop.LoginCloseCallback(std::move(closecb));
}

