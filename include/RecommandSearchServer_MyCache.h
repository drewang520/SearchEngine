#ifndef _RECOMMAND_SEARCH_SERVER_H
#define _RECOMMAND_SEARCH_SERVER_H

#include "Configuration.h"
#include "SocketIO.h"
#include "ProtocolParser.h"
#include "ThreadPool.h"
#include "TcpServer.h"
#include "LRUCache.h"
#include "KeyRecommander.h"
#include "WebPageSearcher.h"
#include <functional>
#include <mutex>
#include <iostream>

using namespace Protocol;
using std::endl;
using std::cout;

class Mytask
: public Task
{
public:
    virtual ~Mytask() {}
    Mytask(const Message&msg, const TcpConnectionPtr& con, const Configuration& config, 
                                LRUCache& cache, std::mutex & mutex)
    : m_msg(msg)
    , m_con(con)
    , m_config(config)
    , m_cache(cache)
    , m_mutex(mutex)
    {
                            
    }

    void process() override
    {
        // msg相应的业务逻辑
        if (m_msg.id == Protocol::KEY_RECOMMAND)
        {
            std::cout << "KEY_RECOMMAND: " << "\n";
            KeyRecommander keyCommander(m_msg.data, m_config);
            {
                std::lock_guard<std::mutex> lockGuard(m_mutex);
                m_msg.data = m_cache.CacheTransaction(m_msg.data, keyCommander);  
            }
            m_cache.getCacheElem();
        }
        else if (m_msg.id == Protocol::WEBPAGE_SEARCH)
        {
            std::cout << "WEBPAGE_SEARCH: " << "\n";
            WebPageSearch webPageSearch(m_msg.data, m_config);
            m_msg.data = ProtocolParser::JsonToString(
                                 ProtocolParser::vecWebToJson(webPageSearch.doQuery()));            
            /* m_msg.data = m_cache.CacheTransaction(m_msg.data, m_webPageSearch); */
        }
        // 处理完毕后将msg返回给EventLoop进行IO操作
        m_con->sendInLoop(m_msg.data);
    }

private:
    Message m_msg;
    TcpConnectionPtr m_con;
    const Configuration& m_config;
    LRUCache & m_cache;
    std::mutex & m_mutex;
};


class RecommandSearchServer
{
public:
    RecommandSearchServer(const Configuration& config)
    : m_threadpool(stoi(config.getConfig().at("threadNums")), stoi(config.getConfig().at("queSize")))
    , m_tcpserver(config.getConfig().at("ip"), stoi(config.getConfig().at("port")))
    , m_config(config)
    , m_cache()
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

        std::unique_ptr<Task> task(new Mytask(recvmsg, con, m_config, m_cache, m_mutex));
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
    LRUCache m_cache;
    std::mutex m_mutex;
};

#endif
