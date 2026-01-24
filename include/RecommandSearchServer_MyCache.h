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
    Mytask(const Message&msg, const TcpConnectionPtr& con, const Configuration * config, 
                                LRUCache& cache, std::mutex & mutex)
    : _msg(msg)
    , _con(con)
    , _config(config)
    , _keyCommander(_msg.data,  _config) 
    , _webPageSearch(_msg.data, _config)
    , _cache(cache)
    , _mutex(mutex)
    {
                            
    }

    void process() override
    {
        // msg相应的业务逻辑
        if (_msg.id == Protocol::KEY_RECOMMAND)
        {
            std::cout << "KEY_RECOMMAND: " << "\n";
            {
                std::lock_guard<std::mutex> lockGuard(_mutex);
                _msg.data = _cache.CacheTransaction(_msg.data, _keyCommander);  
            }
            _cache.getCacheElem();
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
    LRUCache & _cache;
    std::mutex & _mutex;
};


class RecommandSearchServer
{
public:
    RecommandSearchServer(const Configuration * config)
    : _threadpool(stoi(config->getConfig().at("threadNums")), stoi(config->getConfig().at("queSize")))
    , _tcpserver(config->getConfig().at("ip"), stoi(config->getConfig().at("port")))
    , _config(config)
    , _cache()
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
        ProtocolParser::from_json(ProtocolParser::doParse(msg), recvmsg);

        unique_ptr<Task> task(new Mytask(recvmsg, con, _config, _cache, _mutex));
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
    LRUCache _cache;
    std::mutex _mutex;
};

#endif
