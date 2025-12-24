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

using std::vector;
using std::map;
using std::shared_ptr;
using std::function;

using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void (const TcpConnectionPtr& )>; 
using Functor = function<void ()>;

class EventLoop
{
public:
    EventLoop(Acceptor & acceptor);
    ~EventLoop();

    void loop();
    void unloop();

    int createEpollFd();
    void addEpollFd(int fd);
    void delEpollFd(int fd);
    void WaitEpoll();
    void handleNewConnection();
    void handleMessage(int netfd);

    void runEventLoop(Functor &&_cb);
    int createEventFd();
    void handleEventRead();
    void wakeup();
    void doFunctors();

    void LoginConnectionCallback(TcpConnectionCallback && connectioncb);
    void LoginMessageCallback(TcpConnectionCallback && messagecb);
    void LoginCloseCallback(TcpConnectionCallback && closecb);
    

private:
    int _epfd;
    int _evtfd;
    bool _isloop;
    Acceptor & _acceptor;
    vector<struct epoll_event> _readyEvent;    
    map<int, TcpConnectionPtr> _tcpConns;

    vector<Functor> _functor;
    MutexLock _mutex;

    TcpConnectionCallback _connectionCb;
    TcpConnectionCallback _messageCb;
    TcpConnectionCallback _closeCb;
};








#endif
