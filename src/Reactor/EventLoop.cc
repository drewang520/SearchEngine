#include "EventLoop.h"
#include "MutexLock.h"
#include <cstdint>
#include <unistd.h>
#include <stdio.h>
#include <sys/eventfd.h>

EventLoop::EventLoop(Acceptor & acceptor)
: _epfd(createEpollFd())
, _evtfd(createEventFd())
, _isloop(true)
, _acceptor(acceptor)
, _readyEvent(1024)
{
    addEpollFd(_acceptor.getfd());
    addEpollFd(_evtfd);
}

EventLoop::~EventLoop()
{
    if (_epfd)
    {
        close(_epfd);
    }
    if (_evtfd)
    {
        close(_evtfd);
    }
}


void EventLoop::loop()
{
    _isloop = true;
    while (_isloop)
    {
        WaitEpoll();
    }
}

void EventLoop::WaitEpoll()
{
    int readyNum = epoll_wait(_epfd, _readyEvent.data(), 1024, 5000);
    if (-1 == readyNum)
    {
        perror("epoll_wait -1");
        return;
    }
    else if (0 == readyNum)
    {
        printf("connection time out !\n");
    }
    else 
    {
        if (readyNum == (int) _readyEvent.size())
        {
            _readyEvent.resize(2 * readyNum);
        }

        for (int idx = 0; idx < readyNum; ++idx)
        {
            if (_readyEvent[idx].data.fd == _acceptor.getfd())
            {
                handleNewConnection();        
            }
            else if (_readyEvent[idx].data.fd == _evtfd)
            {
                handleEventRead();
                doFunctors();
            }
            else
            {
                handleMessage(_readyEvent[idx].data.fd);
            }
        }
    }
}

void EventLoop::handleNewConnection()
{
    int netfd = _acceptor.accept();
    addEpollFd(netfd);
    TcpConnectionPtr con(new TcpConnection(netfd, this));

    con->LoginConnectionCallback(_connectionCb);
    con->LoginMessageCallback(_messageCb);
    con->LoginCloseCallback(_closeCb);

    _tcpConns.insert(std::make_pair(netfd, con));

    con->ExConnectionCallback();
}

void EventLoop::handleMessage(int netfd)
{
    auto iter = _tcpConns.find(netfd);
    if (iter != _tcpConns.end())
    {
        bool flag = iter->second->isClosed();
        if (flag)
        {
            iter->second->ExCloseCallback();
            delEpollFd(netfd);
            _tcpConns.erase(iter);
        }
        else 
        {
            iter->second->ExMessageCallback();
        }
    }
    else 
    {
        printf("连接不存在！\n");
    }
}

void EventLoop::runEventLoop(Functor && _cb)
{
    {
        MutexLockGuard automutex(_mutex);
        _functor.push_back(_cb);
    }
    
    wakeup();
}

void EventLoop::doFunctors()
{
    vector<Functor> tmp;
    {
        MutexLockGuard automutex(_mutex);
        _functor.swap(tmp);
    }

    for (auto &cb: tmp)
    {
        cb();
    }
}

int EventLoop::createEventFd()
{
    int evtfd = eventfd(10, 0);
    if (-1 == evtfd)
    {
        perror("eventfd -1");
        return -1;
    }
    return evtfd;
}

void EventLoop::handleEventRead()
{
    uint64_t one = 1;
    int ret = ::read(_evtfd, &one, sizeof(one));
    if (ret != sizeof(one))
    {
        perror("read -1");
        return;
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    int ret = ::write(_evtfd, &one, sizeof(one));
    if (ret != sizeof(one))
    {
        perror("write -1");
        return;
    }
}


void EventLoop::unloop()
{
    _isloop = false;
}

int EventLoop::createEpollFd()
{
   _epfd = epoll_create(10);
   if (-1 == _epfd)
   {
       perror("epoll_create -1");
       return -1;
   }
   return _epfd;
}

void EventLoop::addEpollFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &evt);
    if (-1 == ret)
    {
        perror("epoll_ctl -1");
        return;
    }
}

void EventLoop::delEpollFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &evt);
    if (-1 == ret)
    {
        perror("epoll_ctl -1");
        return;
    }
}

void EventLoop::LoginConnectionCallback(TcpConnectionCallback && connectioncb)
{
    _connectionCb = std::move(connectioncb);
}

void EventLoop::LoginMessageCallback(TcpConnectionCallback && messagecb)
{
    _messageCb = std::move(messagecb);
}

void EventLoop::LoginCloseCallback(TcpConnectionCallback && closecb)
{
    _closeCb = std::move(closecb);
}

