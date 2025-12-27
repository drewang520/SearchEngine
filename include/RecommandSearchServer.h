#ifndef _ECHO_SERVER_H_
#define _ECHO_SERVER_H_
#include "SocketIO.h"
#include "ThreadPool.h"
#include "TcpServer.h"
#include "KeyRecommander.h"
#include <functional>
#include <iostream>

using std::endl;
using std::cout;

class Mytask
: public Task
{
public:
    Mytask(const string &msg, const TcpConnectionPtr& con)
    : _msg(msg)
    , _con(con)
    {
                  
    }

    ~Mytask()
    {

    }

    void process() override
    {
        // msg相应的业务逻辑
        /* _msg.append("drewang"); */

        // 处理完毕后将msg返回给EventLoop进行IO操作
        _con->sendInLoop(_msg);
    }

private:
    string _msg;
     TcpConnectionPtr _con;
};


class EchoServer
{
public:
    EchoServer(int threadNums, int quesize, const string& ip, unsigned int port)
    : _threadpool(threadNums, quesize)
    , _tcpserver(ip, port)
    {
        
    }

    void start()
    {
        _threadpool.start();
        _tcpserver.setAllCallback(std::bind(&EchoServer::ConnectionCallback, this, std::placeholders::_1),
                                  std::bind(&EchoServer::MessageCallback, this, std::placeholders::_1),
                                  std::bind(&EchoServer::CloseCallback, this, std::placeholders::_1));
        _tcpserver.start();
    }

    void ConnectionCallback(const  TcpConnectionPtr& con)
    {
        cout << con->toString() << "has connected !" << endl;
    }

    void MessageCallback(const  TcpConnectionPtr& con)
    {
        string msg = con->recvMsg();
        cout << msg << endl; 

        unique_ptr<Task> task(new Mytask(msg, con));
        _threadpool.addTask(std::move(task));
    }

    void CloseCallback(const  TcpConnectionPtr& con)
    {
        cout << con->toString() << "has closed!" << endl;
    }

    ~EchoServer()
    {
        _tcpserver.stop();
        _threadpool.stop();
    }

private:
    ThreadPool _threadpool;
    TcpServer _tcpserver;
};

#endif
