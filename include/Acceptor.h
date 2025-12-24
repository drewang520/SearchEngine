#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "InetAddress.h"
#include "Socket.h"
#include <string>

using std::string;

class Acceptor
{
public:
    Acceptor(const string& ip, unsigned int port);
    ~Acceptor();

    void ready();
    void setReuseAddress();
    void setReusePort();
    void bind();
    void listen();

    int accept();

    int getfd() const;
private:
    Socket _socket;
    InetAddress _addr;
};


#endif
