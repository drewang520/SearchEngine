#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include "Acceptor.h"
#include "MutexLock.h"
#include "TcpConnection.h"
#include <cmath>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <memory>
#include <functional>

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionCallback = std::function<void (const TcpConnectionPtr& )>; 
using Functor = std::function<void ()>;

class EventLoop
{
public:
    EventLoop(Acceptor & acceptor);
    ~EventLoop();

    void loop();
    void unloop();

    int  createEpollFd();
    void addEpollFd(int fd);
    void delEpollFd(int fd);
    void WaitEpoll();
    void handleNewConnection();
    void handleMessage(int netfd);

    void runEventLoop(Functor &&_cb);
    int  createEventFd();
    void handleEventRead();
    void wakeup();
    void doFunctors();

    void LoginConnectionCallback(TcpConnectionCallback && connectioncb);
    void LoginMessageCallback(TcpConnectionCallback && messagecb);
    void LoginCloseCallback(TcpConnectionCallback && closecb);
    

private:
    int m_epfd;
    int m_evtfd;
    bool m_isloop;
    Acceptor & m_acceptor;
    std::vector<struct epoll_event> m_readyEvent;    
    std::map<int, TcpConnectionPtr> m_tcpConns;

    std::vector<Functor> m_functor;
    MutexLock m_mutex;

    TcpConnectionCallback m_connectionCb;
    TcpConnectionCallback m_messageCb;
    TcpConnectionCallback m_closeCb;
};

#endif
