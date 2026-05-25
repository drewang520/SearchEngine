#ifndef _RECOMMAND_SEARCH_SERVER_H
#define _RECOMMAND_SEARCH_SERVER_H

#include "Configuration.h"
#include "SocketIO.h"
#include "ProtocolParser.h"
#include "Thread.h"
#include "ThreadPool.h"
#include "TcpServer.h"
#include "CacheManager.h"
#include "Logger.h"
#include "KeyRecommander.h"
#include "WebPageSearcher.h"
#include "AISearcher.h"
#include <exception>
#include <functional>
#include <string>

using namespace Protocol;

class Mytask
: public Task
{
public:
    Mytask(const Message&msg, const TcpConnectionPtr& con, const Configuration& config, CacheManager& cacheManager)
    : m_msg(msg)
    , m_con(con)
    , m_config(config)
    , m_cacheManager(cacheManager)
    {
                            
    }

    virtual ~Mytask() {}

    void process() override
    {
        // 这里是可以获取到执行该任务的线程id的
        if (m_msg.id == Protocol::KEY_RECOMMAND)
        {
            LOG_INFO(std::string("task=KEY_RECOMMAND thread=") + thread_name
                     + " query=" + m_msg.data);
            // 这里为了减少锁的粒度（临界区的长度）,应该将Cache放入具体的模块之中而不是反过来
            KeyRecommander keyCommander(m_msg.data, m_config);
            const int cacheID = std::stoi(thread_name);
            m_msg.data = keyCommander.startQuery(cacheID);
            auto& workerCache = m_cacheManager.getCache(static_cast<size_t>(cacheID));
            LOG_DEBUG("worker cache size=" + std::to_string(workerCache.getResultList().size())
                      + " cacheID=" + std::to_string(cacheID));
            LOG_DEBUG("KEY_RECOMMAND response bytes=" + std::to_string(m_msg.data.size()));
        }
        else if (m_msg.id == Protocol::WEBPAGE_SEARCH)
        {
            LOG_INFO("task=WEBPAGE_SEARCH query=" + m_msg.data);
            WebPageSearch webPageSearch(m_msg.data, m_config);
            m_msg.data = ProtocolParser::JsonToString(
                                 ProtocolParser::vecWebToJson(webPageSearch.doQuery()));            
            /* m_msg.data = _cache.CacheTransaction(m_msg.data, _webPageSearch); */
            LOG_DEBUG("WEBPAGE_SEARCH response bytes=" + std::to_string(m_msg.data.size()));
        }
        else if (m_msg.id == Protocol::AI_SEARCH)
        {
            AISearcher aiSearcher(m_msg.data, m_config);
            m_msg.data = aiSearcher.doQuery();
        }
        else
        {
            LOG_WARN("unknown request id=" + std::to_string(m_msg.id));
            return;
        }
        // 处理完毕后将msg返回给EventLoop进行IO操作
        m_con->sendInLoop(m_msg.data);
    }

private:
    Message m_msg;
    TcpConnectionPtr m_con;
    const Configuration& m_config;
    CacheManager& m_cacheManager;
};


class RecommandSearchServer
{
public:
    RecommandSearchServer(const Configuration& config)
    : m_threadPool(stoi(config.getConfig().at("threadNums")), stoi(config.getConfig().at("queSize")))
    , m_tcpServer(config.getConfig().at("ip"), stoi(config.getConfig().at("port")))
    , m_config(config)
    , m_cacheManager(CacheManager::createCacheManger())
    {

    }

    void start()
    {
        LOG_INFO("server starting threadNums=" + m_config.getConfig().at("threadNums")
                 + " queueSize=" + m_config.getConfig().at("queSize")
                 + " listen=" + m_config.getConfig().at("ip")
                 + ":" + m_config.getConfig().at("port"));
        m_threadPool.start();
        m_tcpServer.setAllCallback(std::bind(&RecommandSearchServer::ConnectionCallback, this, std::placeholders::_1),
                                  std::bind(&RecommandSearchServer::MessageCallback, this, std::placeholders::_1),
                                  std::bind(&RecommandSearchServer::CloseCallback, this, std::placeholders::_1));
        m_tcpServer.start();
    }

    void ConnectionCallback(const  TcpConnectionPtr& con)
    {
        LOG_INFO("connection open " + con->toString());
    }

    void MessageCallback(const  TcpConnectionPtr& con)
    {
        string msg = con->recvMsg();
        LOG_DEBUG("recv bytes=" + std::to_string(msg.size()) + " from=" + con->toString());
        Message recvmsg;
        try
        {
            ProtocolParser::from_json(ProtocolParser::doParse(msg), recvmsg);
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR(std::string("parse request failed from=") + con->toString()
                      + " reason=" + ex.what());
            return;
        }
        LOG_INFO("request id=" + std::to_string(recvmsg.id)
                 + " length=" + std::to_string(recvmsg.length)
                 + " data=" + recvmsg.data);

        std::unique_ptr<Task> task(new Mytask(recvmsg, con, m_config, m_cacheManager));
        m_threadPool.addTask(std::move(task));
    }

    void CloseCallback(const  TcpConnectionPtr& con)
    {
        LOG_INFO("connection close " + con->toString());
    }

    ~RecommandSearchServer()
    {
        LOG_INFO("server stopping");
        m_tcpServer.stop();
        m_threadPool.stop();
    }

private:
    ThreadPool m_threadPool;
    TcpServer m_tcpServer;
    const Configuration& m_config;
    CacheManager& m_cacheManager;
};

#endif
