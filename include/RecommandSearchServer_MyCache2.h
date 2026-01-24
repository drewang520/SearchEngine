#ifndef _RECOMMAND_SEARCH_SERVER_H
#define _RECOMMAND_SEARCH_SERVER_H

#include "Configuration.h"
#include "SocketIO.h"
#include "ProtocolParser.h"
#include "Thread.h"
#include "ThreadPool.h"
#include "TcpServer.h"
#include "CacheManager.h"
#include "KeyRecommander.h"
#include "WebPageSearcher.h"
#include <functional>
#include <iostream>
#include <pthread.h>
#include <string>

using namespace Protocol;
using std::endl;
using std::cout;

class Mytask
: public Task
{
public:
    Mytask(const Message&msg, const TcpConnectionPtr& con, const Configuration * config, CacheManager * cacheManager)
    : _msg(msg)
    , _con(con)
    , _config(config)
    , _keyCommander(_msg.data,  _config) 
    , _webPageSearch(_msg.data, _config)
    , _cacheManager(cacheManager)
    {
                            
    }

    ~Mytask()
    {

    }

    void process() override
    {
        // 这里是可以获取到执行该任务的线程id的
        if (_msg.id == Protocol::KEY_RECOMMAND)
        {
            std::cout << "KEY_RECOMMAND: " << "\n";
            std::cout << "child thread_name : " << thread_name << "\n"; 
            // 这里为了减少锁的粒度（临界区的长度）,应该将Cache放入具体的模块之中而不是反过来
            _msg.data = _keyCommander.startQuery(std::stoi(thread_name));
            _cacheManager->getCache(std::stoi(thread_name)).getCacheElem();
        }
        else if (_msg.id == Protocol::WEBPAGE_SEARCH)
        {
            std::cout << "WEBPAGE_SEARCH: " << "\n";
            _msg.data = ProtocolParser::JsonToString(
                                 ProtocolParser::vecWebToJson(_webPageSearch.doQuery()));            
            /* _msg.data = _cache.CacheTransaction(_msg.data, _webPageSearch); */
        }
        // 处理完毕后将msg返回给EventLoop进行IO操作
        _con->sendInLoop(_msg.data);
    }

private:
    Message _msg;
    TcpConnectionPtr _con;
    const Configuration * _config;
    KeyRecommander _keyCommander;
    WebPageSearch _webPageSearch;
    CacheManager * _cacheManager;
};


class RecommandSearchServer
{
public:
    RecommandSearchServer(const Configuration * config)
    : _threadpool(stoi(config->getConfig().at("threadNums")), stoi(config->getConfig().at("queSize")))
    , _tcpserver(config->getConfig().at("ip"), stoi(config->getConfig().at("port")))
    , _config(config)
    , _cacheManager(CacheManager::createCacheManger())
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
        cout << msg << "\n"; 
        Message recvmsg;
        ProtocolParser::from_json(ProtocolParser::doParse(msg), recvmsg);

        unique_ptr<Task> task(new Mytask(recvmsg, con, _config, _cacheManager));
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
    const Configuration * _config;
    CacheManager * _cacheManager;
};

#endif
