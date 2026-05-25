#ifndef _RECOMMAND_SEARCH_SERVER_H
#define _RECOMMAND_SEARCH_SERVER_H

#include "Configuration.h"
#include "SocketIO.h"
#include "ProtocolParser.h"
#include "ThreadPool.h"
#include "TcpServer.h"
#include "MyRedis.h"
#include "KeyRecommander.h"
#include "WebPageSearcher.h"
#include <functional>
#include <iostream>

using namespace sw::redis;
using namespace Protocol;
using std::endl;
using std::cout;

class Mytask
: public Task
{
public:
    Mytask(const Message&msg, const TcpConnectionPtr& con, const Configuration& config, MyRedis& myredis)
    : m_msg(msg)
    , m_con(con)
    , m_config(config)
    , m_myredis(myredis)
    {
                            
    }

    ~Mytask()
    {

    }

    void process() override
    {
        // msg相应的业务逻辑
        if (m_msg.id == Protocol::KEY_RECOMMAND)
        {
            std::cout << "KEY_RECOMMAND: " << "\n";
            /* m_msg.data = ProtocolParser::JsonToString( */
            /*                       ProtocolParser::vecToJson(m_keyCommander.doQuery())); */
            KeyRecommander keyCommander(m_msg.data, m_config);
            m_msg.data = m_myredis.RedisTransaction(m_msg.data, keyCommander);  
        }
        else if (m_msg.id == Protocol::WEBPAGE_SEARCH)
        {
            std::cout << "WEBPAGE_SEARCH: " << "\n";
            /* m_msg.data = ProtocolParser::JsonToString( */
            /*                      ProtocolParser::vecWebToJson(m_webPageSearch.doQuery())); */            
            WebPageSearch webPageSearch(m_msg.data, m_config);
            m_msg.data = m_myredis.RedisTransaction(m_msg.data, webPageSearch);
        }
        // 处理完毕后将msg返回给EventLoop进行IO操作
        m_con->sendInLoop(m_msg.data);
    }

private:
    Message m_msg;
    TcpConnectionPtr m_con;
    const Configuration& m_config;
    MyRedis & m_myredis;
};


class RecommandSearchServer
{
public:
    RecommandSearchServer(const Configuration& config, Redis& redis)
    : m_threadpool(stoi(config.getConfig().at("threadNums")), stoi(config.getConfig().at("queSize")))
    , m_tcpserver(config.getConfig().at("ip"), stoi(config.getConfig().at("port")))
    , m_config(config)
    , m_myredis(redis)
    {
        
    }

    void start()
    {
        m_threadpool.start();
        m_tcpserver.setAllCallback(std::bind(&RecommandSearchServer::ConnectionCallback, this, std::placeholders::_1),
                                  std::bind(&RecommandSearchServer::MessageCallback, this, std::placeholders::_1),
                                  std::bind(&RecommandSearchServer::CloseCallback, this, std::placeholders::_1));
        m_tcpserver.start();
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
        ProtocolParser::from_json(ProtocolParser::doParse(msg), recvmsg);
        if (recvmsg.id == KEY_RECOMMAND)
        {
            m_myredis.selectDb(0);            
        }
        else if (recvmsg.id == WEBPAGE_SEARCH)
        {
            m_myredis.selectDb(1);
        }
        std::unique_ptr<Task> task(new Mytask(recvmsg, con, m_config, m_myredis));
        m_threadpool.addTask(std::move(task));
    }

    void CloseCallback(const  TcpConnectionPtr& con)
    {
        cout << con->toString() << "has closed!" << endl;
    }

    ~RecommandSearchServer()
    {
        m_tcpserver.stop();
        m_threadpool.stop();
    }

private:
    ThreadPool m_threadpool;
    TcpServer m_tcpserver;
    const Configuration& m_config;
    MyRedis m_myredis;
};

#endif
