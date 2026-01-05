#ifndef _RECOMMAND_SEARCH_SERVER_H
#define _RECOMMAND_SEARCH_SERVER_H

#include "SocketIO.h"
#include "Configuration.h"
#include "TLVMessage.h"
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
    Mytask(const string &msg, const TcpConnectionPtr& con, Configuration * config)
    : _msg(msg)
    , _con(con)
    , _config(config)
    {
                  
    }

    ~Mytask()
    {

    }

    void process() override
    {
        // msg相应的业务逻辑
        TLV::TLVMessage message;        
        message.decodeMessage(_msg);
        std::cout << "TLV is ok" << "\n";
        if (message.getType() == TLV::MessageType::KEY_COMMANDER)
        {
            KeyRecommander keyrecommander(_msg, _con, _config);
            vector<string> recommandWords = keyrecommander.doQuery();
            for (const auto & word : recommandWords)
            {
                _msg += word;
            }
        }
        else if (message.getType() == TLV::MessageType::PAGE_SEARCHER)
        {
            
        }
        // 处理完毕后将msg返回给EventLoop进行IO操作
        _con->sendInLoop(_msg);
    }

private:
    string _msg;
    TcpConnectionPtr _con;
    Configuration * _config;
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

        unique_ptr<Task> task(new Mytask(msg, con, _config));
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
