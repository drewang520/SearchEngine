#include "EventLoop.h"
#include "Logger.h"
#include "MutexLock.h"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/eventfd.h>

EventLoop::EventLoop(Acceptor & acceptor)
: m_epfd(createEpollFd())
, m_evtfd(createEventFd())
, m_isloop(true)
, m_acceptor(acceptor)
, m_readyEvent(1024)
{
    addEpollFd(m_acceptor.getfd());
    addEpollFd(m_evtfd);
}

EventLoop::~EventLoop()
{
    if (m_epfd)
    {
        close(m_epfd);
    }
    if (m_evtfd)
    {
        close(m_evtfd);
    }
}


void EventLoop::loop()
{
    m_isloop = true;
    while (m_isloop)
    {
        WaitEpoll();
    }
}

void EventLoop::WaitEpoll()
{
    int readyNum = epoll_wait(m_epfd, m_readyEvent.data(), 1024, 5000);
    if (-1 == readyNum)
    {
        LOG_ERROR(std::string("epoll_wait failed: ") + std::strerror(errno));
        return;
    }
    else if (0 == readyNum)
    {
        LOG_DEBUG("epoll wait timeout");
    }
    else 
    {
        if (readyNum == (int) m_readyEvent.size())
        {
            m_readyEvent.resize(2 * readyNum);
        }

        for (int idx = 0; idx < readyNum; ++idx)
        {
            if (m_readyEvent[idx].data.fd == m_acceptor.getfd())
            {
                handleNewConnection();        
            }
            else if (m_readyEvent[idx].data.fd == m_evtfd)
            {
                handleEventRead();
                doFunctors();
            }
            else
            {
                handleMessage(m_readyEvent[idx].data.fd);
            }
        }
    }
}

void EventLoop::handleNewConnection()
{
    int netfd = m_acceptor.accept();
    if (-1 == netfd)
    {
        LOG_ERROR("accept returned -1");
        return;
    }
    addEpollFd(netfd);
    TcpConnectionPtr con(new TcpConnection(netfd, this));

    con->LoginConnectionCallback(m_connectionCb);
    con->LoginMessageCallback(m_messageCb);
    con->LoginCloseCallback(m_closeCb);

    m_tcpConns.insert(std::make_pair(netfd, con));

    LOG_INFO("new connection fd=" + std::to_string(netfd));
    con->ExConnectionCallback();
}

void EventLoop::handleMessage(int netfd)
{
    auto iter = m_tcpConns.find(netfd);
    if (iter != m_tcpConns.end())
    {
        bool flag = iter->second->isClosed();
        if (flag)
        {
            LOG_INFO("connection closed fd=" + std::to_string(netfd));
            iter->second->ExCloseCallback();
            delEpollFd(netfd);
            m_tcpConns.erase(iter);
        }
        else 
        {
            iter->second->ExMessageCallback();
        }
    }
    else 
    {
        LOG_WARN("connection fd not found fd=" + std::to_string(netfd));
    }
}

void EventLoop::runEventLoop(Functor && _cb)
{
    {
        MutexLockGuard automutex(m_mutex);
        m_functor.push_back(_cb);
    }
    
    wakeup();
}

void EventLoop::doFunctors()
{
    std::vector<Functor> tmp;
    {
        MutexLockGuard automutex(m_mutex);
        m_functor.swap(tmp);
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
        LOG_ERROR(std::string("eventfd failed: ") + std::strerror(errno));
        return -1;
    }
    return evtfd;
}

void EventLoop::handleEventRead()
{
    uint64_t one = 1;
    int ret = ::read(m_evtfd, &one, sizeof(one));
    if (ret != sizeof(one))
    {
        LOG_ERROR(std::string("eventfd read failed: ") + std::strerror(errno));
        return;
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    int ret = ::write(m_evtfd, &one, sizeof(one));
    if (ret != sizeof(one))
    {
        LOG_ERROR(std::string("eventfd write failed: ") + std::strerror(errno));
        return;
    }
}


void EventLoop::unloop()
{
    m_isloop = false;
}

int EventLoop::createEpollFd()
{
   m_epfd = epoll_create(10);
   if (-1 == m_epfd)
   {
       LOG_ERROR(std::string("epoll_create failed: ") + std::strerror(errno));
       return -1;
   }
   return m_epfd;
}

void EventLoop::addEpollFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &evt);
    if (-1 == ret)
    {
        LOG_ERROR(std::string("epoll_ctl add failed fd=") + std::to_string(fd)
                  + " reason=" + std::strerror(errno));
        return;
    }
}

void EventLoop::delEpollFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &evt);
    if (-1 == ret)
    {
        LOG_ERROR(std::string("epoll_ctl del failed fd=") + std::to_string(fd)
                  + " reason=" + std::strerror(errno));
        return;
    }
}

void EventLoop::LoginConnectionCallback(TcpConnectionCallback && connectioncb)
{
    m_connectionCb = std::move(connectioncb);
}

void EventLoop::LoginMessageCallback(TcpConnectionCallback && messagecb)
{
    m_messageCb = std::move(messagecb);
}

void EventLoop::LoginCloseCallback(TcpConnectionCallback && closecb)
{
    m_closeCb = std::move(closecb);
}
