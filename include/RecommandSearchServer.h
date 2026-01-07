#ifndef _RECOMMAND_SEARCH_SERVER_H
#define _RECOMMAND_SEARCH_SERVER_H

#include "SocketIO.h"
#include "Configuration.h"
#include "ProtocolParser.h"
#include "ThreadPool.h"
#include "TcpServer.h"
#include "KeyRecommander.h"
#include <functional>
#include <iostream>

using namespace Protocol;
using std::endl;
using std::cout;

class Mytask
: public Task
{
public:
    Mytask(const Message&msg, const TcpConnectionPtr& con, Configuration * config)
    : _msg(msg)
    , _con(con)
    , _config(config)
    , _keyCommander(_msg.data, _con, config) 
    {
                  
    }

    ~Mytask()
    {

    }

    void process() override
    {
        // msg相应的业务逻辑
        if (_msg.id == 1)
        {
            _msg.data = ProtocolParser::JsonToString(
                                  ProtocolParser::vecToJson(_keyCommander.doQuery()));
        }
        else if (_msg.id == 2)
        {

        }
        // 处理完毕后将msg返回给EventLoop进行IO操作
        _con->sendInLoop(_msg.data);
    }

private:
    Message _msg;
    TcpConnectionPtr _con;
    Configuration * _config;
    KeyRecommander _keyCommander;
};


class RecommandSearchServer
{
public:
    RecommandSearchServer(Configuration * config)
    : _threadpool(stoi(config->getConfig()["threadNums"]), stoi(config->getConfig()["queSize"]))
    , _tcpserver(config->getConfig()["ip"], stoi(config->getConfig()["port"]))
    , _config(config)
    {
        
    }

    void start()
    {
        _threadpool.start();
        _tcpserver.setAllCallback(std::bind(&RecommandSearchServer::ConnectionCallback, this, std::placeholders::_1),
                                  std::bind(&RecommandSearchServer::MessageCallback, this, std::placeholders::_1),
                                  std::bind(&RecommandSearchServer::CloseCallback, this, std::placeholders::_1));
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
        Message recvmsg;
        ProtocolParser::from_json(ProtocolParser::daParse(msg), recvmsg);

        unique_ptr<Task> task(new Mytask(recvmsg, con, _config));
        _threadpool.addTask(std::move(task));
    }

    void CloseCallback(const  TcpConnectionPtr& con)
    {
        cout << con->toString() << "has closed!" << endl;
    }

    ~RecommandSearchServer()
    {
        _tcpserver.stop();
        _threadpool.stop();
    }

private:
    ThreadPool _threadpool;
    TcpServer _tcpserver;
    Configuration * _config;
};

#endif
