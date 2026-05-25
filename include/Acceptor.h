#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "InetAddress.h"
#include "Socket.h"
#include <string>

class Acceptor
{
public:
    Acceptor(const std::string& ip, unsigned int port);
    ~Acceptor();

    void ready();
    void setReuseAddress();
    void setReusePort();
    void bind();
    void listen();

    int accept();
    int getfd() const;
    
private:
    Socket m_socket;
    InetAddress m_addr;
};


#endif
