# 搜索引擎项目文档

本文档用于帮助重新熟悉本项目的整体结构、核心组件、对应源文件、功能职责以及完整运行流程。

## 1. 项目概览

这是一个基于 C++ 实现的简易搜索引擎项目，整体分为两条主线：

1. 离线建库流程
   - 抓取或读取原始语料。
   - 构建关键词推荐所需的词典和字索引。
   - 解析 RSS/XML 网页语料，生成网页库。
   - 对网页去重。
   - 对去重后的网页库分词并构建倒排索引。

2. 在线查询流程
   - 客户端向服务器发送 JSON 格式请求。
   - 服务器通过 Reactor 网络层接收连接和消息。
   - 工作线程处理关键词推荐或网页搜索任务。
   - 查询结果通过事件循环回写给客户端。
   - 关键词推荐支持本地 LRU 缓存和 Redis 缓存两个方向的实现。

项目主要技术点：

- C++17
- CMake
- Reactor + epoll + eventfd
- 线程池
- cppjieba 中文分词
- Simhash 网页去重
- TF-IDF + 余弦相似度网页排序
- 编辑距离关键词推荐
- nlohmann/json 消息序列化
- tinyxml2 XML 解析
- 本地 LRU 缓存和 Redis 缓存

## 2. 目录结构

```text
.
├── CMakeLists.txt
├── config/
│   └── config.json
├── data/
│   ├── cache_data/
│   ├── dic_data/
│   ├── html/
│   └── page_data/
├── include/
│   ├── *.h
│   ├── cppjieba/
│   ├── dict/
│   ├── nlohmann/
│   ├── pybind11/
│   └── simhash/
├── raw_data/
│   ├── module1/
│   └── module2/
└── src/
    ├── *.cc
    ├── Reactor/
    └── crawl/
```

核心目录说明：

| 目录 | 作用 |
| --- | --- |
| `config/` | 项目配置文件，包含服务端地址、端口、线程数、缓存参数、语料路径、索引路径等。 |
| `raw_data/` | 原始语料。`module1` 偏向词典语料，`module2` 偏向网页/XML 语料。 |
| `data/dic_data/` | 离线生成的词典和词典索引文件。 |
| `data/page_data/` | 离线生成的网页库、网页 offset 表、倒排索引表。 |
| `data/cache_data/` | 本地缓存持久化文件。 |
| `data/html/` | 客户端展示搜索结果使用的简单 HTML 页面。 |
| `include/` | 项目头文件以及第三方头文件库。 |
| `src/` | 项目源文件。 |
| `src/Reactor/` | 网络层、线程池、同步原语等实现。 |
| `src/crawl/` | 爬虫相关 C++/Python 代码。 |
| `build/` | CMake 构建输出目录。 |

## 3. 构建和运行约定

### 3.1 构建方式

项目使用 CMake 构建，根目录下的 `CMakeLists.txt` 定义了多个可执行目标。

常用构建命令：

```bash
cmake --build build
```

如果需要重新生成构建目录：

```bash
cmake -S . -B build
cmake --build build
```

### 3.2 运行目录注意事项

当前配置读取路径写在 `src/Configuration.cc` 中：

```cpp
string Configuration::_filepath = "../config/config.json";
```

因此可执行文件默认应从 `build/` 目录运行。例如：

```bash
cd build
./RSserver_Cache2
./client
```

如果从项目根目录直接运行 `./build/Config_test` 这类目标，程序会尝试访问 `../config/config.json`，从而找不到配置文件。

### 3.3 CMake 目标

| 目标 | 主要源文件 | 功能 |
| --- | --- | --- |
| `crawl` | `src/crawl/crawl.cc`, `src/crawl/test_crawl.cc`, `src/tinyxml2.cpp` | 通过嵌入 Python 抓取 XML/RSS 数据。 |
| `client` | `src/clientTest.cc` 等 | 交互式客户端，连接搜索服务器。 |
| `Config_test` | `src/test_config.cc` | 测试配置文件读取。 |
| `dicProducer` | `src/DicProducer.cc`, `src/testDicProducer.cc` | 构建关键词推荐词典和字索引。 |
| `pageLibPreprocessor_test` | `src/Optimize_pageLibPreprocessor.cc`, `src/pageLibPreprocessor_test.cc` | 构建网页倒排索引。 |
| `PageProducer_test` | `src/PageProducer.cc`, `src/test_pageProducer.cc` | 解析 RSS/XML，生成网页库并去重。 |
| `WebPageSearch` | `src/WebPageQuery.cc`, `src/test_WebPageSearch.cc` | 测试网页搜索模块。 |
| `KeyCommander_test` | `src/KeyRecommander.cc`, `src/test_KeyRecommander.cc` | 测试关键词推荐模块。 |
| `dictionary` | `src/Dictionary.cc`, `src/testDictionary.cc` | 测试词典加载。 |
| `RSserver_Cache` | `src/Reactor/TestRSserver_MyCache.cc` 等 | 使用单个本地 LRUCache 的服务端版本。 |
| `RSserver_Cache2` | `src/Reactor/TestRSserver_MyCache2.cc` 等 | 使用多工作线程缓存和定时合并的服务端版本。 |
| `RSserver_Myredis` | `src/Reactor/TestRSserver_Myredis.cc`, `src/MyRedis.cc` 等 | 使用 Redis 作为缓存的服务端版本。 |

## 4. 配置文件

### 4.1 对应文件

- `config/config.json`
- `include/Configuration.h`
- `src/Configuration.cc`

### 4.2 功能说明

`Configuration` 是单例类，负责：

- 读取 `config/config.json`。
- 将 JSON 中的字符串和数字配置统一保存到 `map<string, string>`。
- 加载英文停用词、中文停用词、cppjieba 停用词。
- 向其他模块提供配置项和停用词集合。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `Configuration::createpInstance()` | 获取全局配置单例。 |
| `getConfig()` | 获取配置表。 |
| `getStopWords()` | 获取停用词集合。 |

### 4.3 主要配置项

| 配置项 | 作用 |
| --- | --- |
| `ip`, `port` | 搜索服务器监听地址和端口。 |
| `threadNums` | 工作线程数量。 |
| `queSize` | 线程池任务队列容量。 |
| `candidataNum` | 关键词推荐返回候选数量。 |
| `queryWebPageNum` | 网页搜索返回结果数量。 |
| `cacheSize` | 单个 LRU 缓存容量。 |
| `delay`, `interval` | 定时器首次触发延迟和间隔。 |
| `writeCachedelay`, `writeCacheinterval` | 缓存写盘相关参数。 |
| `host`, `redis_port` | Redis 连接参数。 |
| `dict_path`, `model_path`, `user_dict_path`, `idf_path`, `stop_word_path` | cppjieba 分词模型和词典路径。 |
| `En_dict`, `Cn_dict` | 构建英文/中文推荐词典的原始语料路径。 |
| `dic*.dat`, `dicIndex*.dat` | 关键词推荐词典和索引文件路径。 |
| `page_src` | RSS/XML 网页语料目录。 |
| `ripepage`, `pageoffset` | 原始网页库和 offset 表。 |
| `newripepage`, `newoffset` | 去重后的网页库和 offset 表。 |
| `invertIndexTable` | 网页搜索倒排索引表。 |
| `cacheData` | 本地 LRU 缓存持久化文件。 |

## 5. 分词组件

### 5.1 对应文件

- `include/SplitWord.h`
- `include/CppJieBaSplit.h`
- `src/CppJiebaSplit.cc`
- 第三方头文件：`include/cppjieba/`
- 分词模型：`raw_data/module1/dict/` 或 `include/dict/`

### 5.2 功能说明

`SplitWord` 是一个抽象分词接口，目前项目中真正使用的是 `CppJiebaSplit`。

`CppJiebaSplit` 封装 cppjieba，负责：

- 初始化 cppjieba 分词器。
- 对输入文本进行中文分词。
- 过滤停用词。
- 支持两种输出形式：
  - `vector<string>`：用于查询词拆分、网页内容拆分。
  - `map<string, int>`：用于统计词频。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `cut(const string&, vector<string>&)` | 分词并输出词列表。 |
| `cut(const string&, map<string, int>&)` | 分词并统计词频。 |

## 6. 关键词推荐词典组件

### 6.1 离线构建组件

对应文件：

- `include/DicProducer.h`
- `src/DicProducer.cc`
- `src/testDicProducer.cc`

生成数据：

- `data/dic_data/dicEn.dat`
- `data/dic_data/dicCn.dat`
- `data/dic_data/dicindexEn.dat`
- `data/dic_data/dicindexCn.dat`
- `data/dic_data/dic.dat`
- `data/dic_data/dicIndex.dat`

`DicProducer` 负责从原始语料构建关键词推荐使用的词典和字索引。

英文词典流程：

1. 读取 `En_dict` 指定的英文文本。
2. 将非字母字符替换为空格。
3. 将大写字母转小写。
4. 过滤停用词。
5. 统计每个英文单词词频。
6. 为每个字符建立字母到词典下标的索引。

中文词典流程：

1. 遍历 `Cn_dict` 指定的中文语料目录。
2. 使用 `CppJiebaSplit` 分词。
3. 统计中文词频。
4. 按 UTF-8 字符切分每个中文词。
5. 建立“单字到词典下标集合”的索引。

合并流程：

1. 分别读取中英文词典。
2. 将中文词典和英文词典合并为 `dic.dat`。
3. 合并中英文索引为 `dicIndex.dat`。
4. 英文索引下标会加上中文词典长度，避免下标冲突。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `buildEnDict()` | 构建英文词典。 |
| `buildCnDict()` | 构建中文词典。 |
| `createEnIndex()` | 构建英文字母索引。 |
| `createCnIndex()` | 构建中文字索引。 |
| `storeDict()` | 保存单个词典。 |
| `storeIndex()` | 保存单个索引。 |
| `buildDictAndIndex()` | 合并中英文词典和索引。 |
| `storeAllDict()` | 保存合并词典。 |
| `storeALlIndex()` | 保存合并索引。 |

### 6.2 在线加载组件

对应文件：

- `include/Dictionary.h`
- `src/Dictionary.cc`
- `src/testDictionary.cc`

`Dictionary` 负责加载离线生成的词典和索引：

- 从 `dic.dat` 加载 `vector<pair<string, int>>`。
- 从 `dicIndex.dat` 加载 `map<string, set<int>>`。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `getDict()` | 获取词典数组，元素为 `词语 + 词频`。 |
| `getIndex()` | 获取字索引，元素为 `字/字符 + 词典下标集合`。 |

## 7. 关键词推荐查询组件

### 7.1 对应文件

- `include/KeyRecommander.h`
- `src/KeyRecommander.cc`
- `src/test_KeyRecommander.cc`

### 7.2 功能说明

`KeyRecommander` 负责根据用户输入给出候选推荐词。

核心思路：

1. 将用户输入按 UTF-8 字符拆分。
2. 对每个字符查询 `dicIndex.dat` 中的字索引。
3. 合并所有候选词。
4. 计算候选词和查询词之间的编辑距离。
5. 使用优先队列排序。
6. 按配置项 `candidataNum` 返回前 N 个推荐词。

排序规则由 `CompareHot` 决定：

1. 编辑距离越小越靠前。
2. 编辑距离相同，词频越高越靠前。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `doQuery()` | 执行关键词推荐，返回字符串列表。 |
| `startQuery(int cacheID)` | 带本地缓存的查询入口。 |
| `editDistance()` | 计算中英文混合 UTF-8 字符串编辑距离。 |
| `queryIndex()` | 根据单个字符从字索引中查候选词。 |
| `candidataSort()` | 候选词排序并截断。 |

## 8. 网页库生成组件

### 8.1 爬虫组件

对应文件：

- `include/crawl.h`
- `src/crawl/crawl.cc`
- `src/crawl/obtain_xml.py`
- `src/crawl/test_crawl.cc`

`Crawl_XML` 通过 pybind11 嵌入 Python，调用 `obtain_xml.py` 中的 `download_xml()` 下载 XML/RSS。

主要功能：

- 下载站点地图。
- 解析站点地图中的分类链接。
- 根据分类链接下载 RSS XML。
- 将抓取结果写入 `raw_data/module2/people_yuliao2/cn/`。

注意：

- 当前 `crawl.cc` 中包含硬编码绝对路径，例如 Python 模块路径和数据保存路径。
- 如果迁移项目目录，需要同步调整这些路径。

### 8.2 网页库构建组件

对应文件：

- `include/PageProducer.h`
- `src/PageProducer.cc`
- `src/test_pageProducer.cc`
- `include/tinyxml2.h`
- `src/tinyxml2.cpp`

生成数据：

- `data/page_data/ripepage.dat`
- `data/page_data/pageoffset.dat`
- `data/page_data/newripepage.dat`
- `data/page_data/newoffset.dat`

`PageProducer` 负责从 RSS/XML 文件中提取网页信息，生成搜索引擎内部使用的网页库。

处理流程：

1. 遍历 `page_src` 指定的 XML 目录。
2. 使用 tinyxml2 解析每个 RSS 文件。
3. 提取 `title`、`link`、`description`。
4. 对特殊 XML 文件做字段兼容：
   - `coolshell.xml` 使用 `content:encoded`。
   - `dataparse.xml` 和 `latest.xml` 使用 `content`。
5. 使用正则清理 HTML 标签、换行、空格、播放器脚本等。
6. 写入 `<doc>` 格式的网页库。
7. 记录每篇文档在网页库文件中的起始位置和长度。
8. 使用 Simhash 对网页内容去重。
9. 生成去重后的网页库和 offset 表。

网页库中的单篇文档格式：

```xml
<doc>
    <docid>1</docid>
    <title>...</title>
    <link>...</link>
    <content>...</content>
</doc>
```

主要接口：

| 接口 | 作用 |
| --- | --- |
| `store()` | 保存原始网页库和 offset 表。 |
| `pageDeduplicat()` | 使用 Simhash 去重，并保存去重网页库。 |
| `create()` | 解析单个 XML 文件。 |

## 9. 网页倒排索引组件

### 9.1 对应文件

- `include/pageLibPreprocessor.h`
- `src/Optimize_pageLibPreprocessor.cc`
- `src/pageLibPreprocessor.cc`
- `src/pageLibPreprocessor_test.cc`

生成数据：

- `data/page_data/invertIndexTable.dat`

### 9.2 功能说明

`pageLibPreprocessor` 负责对去重后的网页库建立倒排索引。

当前 CMake 中实际使用的是：

- `src/Optimize_pageLibPreprocessor.cc`

`src/pageLibPreprocessor.cc` 是早期版本实现，接口与当前头文件不完全一致，可以视为历史代码。

### 9.3 处理流程

1. 读取 `newoffset`，得到每篇文档在 `newripepage` 中的位置和长度。
2. 逐篇读取 `<doc>` 文档。
3. 使用 tinyxml2 解析文档。
4. 提取 `docid` 和 `content`。
5. 使用 `CppJiebaSplit` 对正文分词。
6. 统计每篇文档中的词频 TF。
7. 统计每个词出现在哪些文档中，得到文档频率 DF。
8. 计算权重：

   ```text
   weight = TF * IDF
   IDF = log2(pageNum / (docFrequency + 1))
   ```

9. 按文档归一化权重。
10. 保存倒排索引。

倒排索引文件格式：

```text
word docid weight docid weight ...
```

主要接口：

| 接口 | 作用 |
| --- | --- |
| `buildInvertIndexMap()` | 构建倒排索引。 |
| `store()` | 保存倒排索引到 `invertIndexTable`。 |
| `dealContent()` | 处理单篇文档分词结果，更新 TF/DF。 |

## 10. 网页搜索组件

### 10.1 对应文件

- `include/WebPageSearcher.h`
- `src/WebPageQuery.cc`
- `include/WebPage.h`
- `src/WebPage.cc`
- `src/test_WebPageSearch.cc`

### 10.2 类职责

| 类 | 作用 |
| --- | --- |
| `WebPage` | 表示一篇网页文档，负责从 `<doc>` XML 字符串中解析 `docid/title/link/content`。 |
| `WebPageQuery` | 加载倒排索引和 offset 表，执行实际网页检索。 |
| `WebPageSearch` | 对外包装类，持有查询词、分词器和 `WebPageQuery`。 |

### 10.3 查询流程

1. `WebPageSearch::doQuery()` 调用 `WebPageQuery::doQuery()`。
2. 使用 `CppJiebaSplit` 对用户查询词分词。
3. 对每个查询词在倒排索引中查找包含该词的文档。
4. 构造每篇候选文档在查询词维度上的权重向量。
5. 构造查询词本身的权重向量。
6. 计算查询向量和每篇文档向量之间的余弦相似度。
7. 按相似度从高到低排序。
8. 根据 `queryWebPageNum` 取前 N 篇文档。
9. 使用 offset 表从 `newripepage` 中读取文档内容。
10. 构造 `WebPage` 对象并返回给上层。

返回给客户端时，当前只序列化：

- `title`
- `link`

## 11. 协议和 JSON 序列化组件

### 11.1 JSON 协议组件

对应文件：

- `include/ProtocolParser.h`
- `src/ProtocolParser.cc`

在线通信当前实际使用 JSON 文本。

请求消息结构：

```cpp
struct Message
{
    int id;
    int length;
    string data;
};
```

消息类型：

| 枚举 | 含义 |
| --- | --- |
| `KEY_RECOMMAND` | 关键词推荐请求。 |
| `WEBPAGE_SEARCH` | 网页搜索请求。 |

主要接口：

| 接口 | 作用 |
| --- | --- |
| `to_json(Message)` | 将请求消息转为 JSON。 |
| `from_json(Message)` | 从 JSON 解析请求消息。 |
| `doParse()` | 将字符串解析为 JSON。 |
| `JsonToString()` | 将 JSON dump 成字符串。 |
| `vecToJson()` | 推荐词列表转 JSON。 |
| `jsonToVec()` | JSON 转推荐词列表。 |
| `vecWebToJson()` | 网页结果列表转 JSON。 |
| `jsonToVecWeb()` | JSON 转网页结果列表。 |

### 11.2 TLV 协议组件

对应文件：

- `include/TLVMessage.h`
- `src/TLVMessage.cc`

`TLVMessage` 实现了 Type-Length-Value 消息格式：

- Type：1 字节。
- Length：2 字节。
- Value：变长内容。

当前 `SocketIO.h` 中有相关 include 和注释，但在线通信主流程并没有真正使用 TLV，而是使用以换行分隔的 JSON 文本。

## 12. 网络层 Reactor 组件

### 12.1 对应文件

- `include/Socket.h`
- `src/Reactor/Socket.cc`
- `include/InetAddress.h`
- `src/Reactor/InetAddress.cc`
- `include/Acceptor.h`
- `src/Reactor/Acceptor.cc`
- `include/SocketIO.h`
- `src/Reactor/SocketIO.cc`
- `include/TcpConnection.h`
- `src/Reactor/TcpConnection.cc`
- `include/EventLoop.h`
- `src/Reactor/EventLoop.cc`
- `include/TcpServer.h`
- `src/Reactor/TcpServer.cc`

### 12.2 组件职责

| 组件 | 作用 |
| --- | --- |
| `Socket` | RAII 封装 socket 文件描述符。 |
| `InetAddress` | 封装 IP 和端口。 |
| `Acceptor` | 创建监听 socket，设置地址复用，bind/listen/accept。 |
| `SocketIO` | 封装定长读、按行读、定长写。 |
| `TcpConnection` | 表示一个客户端连接，负责收发消息和触发回调。 |
| `EventLoop` | Reactor 核心，基于 epoll 监听连接、消息和 eventfd。 |
| `TcpServer` | 对 `Acceptor + EventLoop` 的上层封装。 |

### 12.3 EventLoop 工作方式

`EventLoop` 监听两类 fd：

1. 监听 socket fd
   - 有新连接时调用 `handleNewConnection()`。
   - 创建 `TcpConnection`。
   - 注册连接、消息、关闭回调。

2. 客户端连接 fd
   - 有消息时调用 `handleMessage()`。
   - 如果连接关闭，执行关闭回调并从 epoll 删除。
   - 如果仍有数据，执行消息回调。

3. eventfd
   - 工作线程处理完任务后，不能直接在工作线程中执行 IO 线程的发送操作。
   - `TcpConnection::sendInLoop()` 将发送操作包装成 functor 投递给 `EventLoop`。
   - `EventLoop::wakeup()` 写 eventfd 唤醒 epoll。
   - IO 线程在 `doFunctors()` 中执行真正发送。

## 13. 线程池和同步组件

### 13.1 对应文件

- `include/Thread.h`
- `src/Reactor/Thread.cc`
- `include/WorkThread.h`
- `include/ThreadPool.h`
- `src/Reactor/ThreadPool.cc`
- `include/Task.h`
- `include/TaskQueue.h`
- `src/Reactor/TaskQueue.cc`
- `include/MutexLock.h`
- `src/Reactor/MutexLock.cc`
- `include/Condition.h`
- `src/Reactor/Condition.cc`
- `include/NoCopyable.h`

### 13.2 组件职责

| 组件 | 作用 |
| --- | --- |
| `Task` | 任务抽象基类，定义 `process()`。 |
| `TaskQueue` | 有界阻塞任务队列，使用互斥锁和条件变量实现生产消费。 |
| `Thread` | pthread 封装，提供 `start()` 和 `end()`。 |
| `WorkThread` | 工作线程，调用线程池的 `threadFunc()` 循环取任务。 |
| `TimerThread` | 定时器线程，调用 `TimerManager::start()`。 |
| `ThreadPool` | 管理工作线程、定时器线程和任务队列。 |
| `MutexLock` | pthread mutex RAII 封装。 |
| `MutexLockGuard` | 自动加锁解锁。 |
| `Condition` | pthread condition 封装。 |
| `NoCopyable` | 禁止拷贝的基类。 |

### 13.3 线程池流程

1. 服务端启动时调用 `_threadpool.start()`。
2. 创建 `threadNums` 个 `WorkThread`。
3. 额外创建 1 个 `TimerThread`。
4. `WorkThread` 不断从 `TaskQueue` 中 `pop()` 任务。
5. 网络线程收到消息后创建任务并调用 `_threadpool.addTask()`。
6. 工作线程执行任务的 `process()`。
7. 任务处理完成后通过 `TcpConnection::sendInLoop()` 将发送操作投递回 EventLoop。

## 14. 缓存组件

### 14.1 本地 LRU 缓存

对应文件：

- `include/LRUCache.h`
- `src/LRUCache.cc`

`LRUCache` 负责缓存关键词推荐结果。

内部数据结构：

- `list<pair<string, json>> _resultList`
  - 保存缓存内容。
  - 越靠前表示越新。
- `unordered_map<string, list<...>::iterator> _hashMap`
  - 根据查询词快速定位链表节点。
- `list<pair<string, json>> _pendingUpdateList`
  - 当前工作线程缓存中发生过访问或新增的记录，用于后续同步到主缓存。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `get()` | 查询缓存，命中时移动到链表头。 |
| `addElem()` | 添加缓存元素，超过容量则淘汰最久未使用元素。 |
| `CacheTransaction()` | 单缓存版本的查询事务。 |
| `readFromFile()` | 从磁盘加载缓存。 |
| `writeToFile()` | 将缓存写回磁盘。 |
| `update()` | 用主缓存更新工作线程缓存。 |

### 14.2 多工作线程缓存管理

对应文件：

- `include/CacheManager.h`
- `src/CacheManager.cc`

`CacheManager` 是单例，负责管理：

- 一个主缓存 `_mainCache`。
- 每个工作线程一个独立缓存 `_caches`。
- 每个工作线程缓存对应一把 mutex。

设计意图：

1. 启动时从 `cacheData` 加载主缓存。
2. 每个工作线程缓存以主缓存为初始副本。
3. 工作线程查询自己的缓存，减少全局锁竞争。
4. 定时器周期性将各工作线程的 pending update 合并到主缓存。
5. 再用主缓存刷新所有工作线程缓存。
6. 周期性写盘。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `createCacheManger()` | 获取全局缓存管理器。 |
| `getCache(size_t)` | 获取某个工作线程的缓存。 |
| `getMutex(size_t)` | 获取对应缓存的互斥锁。 |
| `PeriodicalUpdateCache()` | 周期性合并工作线程缓存到主缓存。 |
| `getMainCache()` | 获取主缓存。 |

### 14.3 定时器组件

对应文件：

- `include/TimerManager.h`
- `src/TimerManager.cc`

`TimerManager` 使用 `timerfd` 实现周期性任务。

相关任务：

| 类 | 作用 |
| --- | --- |
| `TimerTask` | 定时任务抽象基类。 |
| `CacheUpdateTask` | 调用 `CacheManager::PeriodicalUpdateCache()`。 |
| `WriteCacheTask` | 将主缓存写入磁盘。 |
| `TimerManager` | 管理 timerfd 和定时任务列表。 |

当前主要流程：

1. `TimerThread` 调用 `TimerManager::start()`。
2. `TimerManager` 创建 `CacheUpdateTask`。
3. timerfd 到期后触发缓存合并。
4. 根据写盘计时决定是否调用 `writeToFile()`。

### 14.4 Redis 缓存

对应文件：

- `include/MyRedis.h`
- `src/MyRedis.cc`
- `include/RecommandSearchServer_Myredis.h`
- `src/Reactor/TestRSserver_Myredis.cc`

`MyRedis` 基于 redis-plus-plus 封装 Redis 查询缓存。

功能：

- 关键词推荐结果缓存。
- 网页搜索结果缓存。
- 查询时先查 Redis。
- 未命中时进入临界区再次检查 Redis。
- 仍未命中才执行真实查询并写入 Redis。

主要接口：

| 接口 | 作用 |
| --- | --- |
| `RedisTransaction(queryWord, KeyRecommander&)` | 关键词推荐 Redis 查询事务。 |
| `RedisTransaction(queryWord, WebPageSearch&)` | 网页搜索 Redis 查询事务。 |
| `selectDb(unsigned int)` | 选择 Redis DB。 |

## 15. 搜索服务器组件

项目中有三套服务端包装头文件，它们的网络层相同，缓存策略不同。

### 15.1 单 LRUCache 服务端

对应文件：

- `include/RecommandSearchServer_MyCache.h`
- `src/Reactor/TestRSserver_MyCache.cc`

特点：

- 服务端内部持有一个 `LRUCache`。
- 关键词推荐查询时使用一把全局 mutex 保护缓存。
- 实现简单，但并发时锁粒度较大。

### 15.2 多工作线程缓存服务端

对应文件：

- `include/RecommandSearchServer_MyCache2.h`
- `src/Reactor/TestRSserver_MyCache2.cc`

特点：

- 服务端使用 `CacheManager`。
- 每个工作线程使用独立 LRUCache。
- 使用 `thread_local const char * thread_name` 作为工作线程 ID。
- 定时器线程周期性合并各线程缓存。
- 这是项目当前比较完整的本地缓存版本。

处理逻辑：

1. `MessageCallback()` 从连接读取 JSON。
2. `ProtocolParser` 解析为 `Message`。
3. 创建 `Mytask`。
4. 将任务放入线程池。
5. 工作线程执行：
   - `KEY_RECOMMAND`：调用 `KeyRecommander::startQuery(cacheID)`。
   - `WEBPAGE_SEARCH`：调用 `WebPageSearch::doQuery()`。
6. 任务完成后调用 `_con->sendInLoop(_msg.data)` 返回结果。

### 15.3 Redis 服务端

对应文件：

- `include/RecommandSearchServer_Myredis.h`
- `src/Reactor/TestRSserver_Myredis.cc`
- `src/MyRedis.cc`

特点：

- Redis DB 0 用于关键词推荐。
- Redis DB 1 用于网页搜索。
- 查询时先查 Redis，未命中时再执行实际搜索。
- 需要本机或远端 Redis 服务可用。

## 16. 客户端组件

### 16.1 交互式客户端

对应文件：

- `src/clientTest.cc`
- `data/html/client.html`
- `data/html/error.html`
- `data/html/index.html`

功能：

1. 读取配置中的服务器 IP 和端口。
2. 创建 TCP socket 并连接服务端。
3. 用户输入关键词。
4. 构造 `KEY_RECOMMAND` JSON 请求。
5. 接收推荐词列表。
6. 用户从推荐词中选择搜索词。
7. 构造 `WEBPAGE_SEARCH` JSON 请求。
8. 接收网页搜索结果。
9. 将结果写入 `data/html/client.html`。
10. 调用浏览器打开 HTML 页面。

当前通信方式：

- 客户端发送 JSON 字符串，并在末尾追加换行。
- 服务端使用 `SocketIO::readLine()` 按行读取。
- 服务端返回 JSON 字符串。
- 客户端用固定大小缓冲区接收。

### 16.2 批量客户端启动器

对应文件：

- `src/Main_client.cc`

功能：

- fork 出多个 `client` 子进程。
- 父进程从终端读取输入。
- 通过 pipe 将同一行输入广播给所有客户端。
- 用于模拟多个客户端并发输入。

## 17. 辅助和第三方组件

| 文件/目录 | 作用 |
| --- | --- |
| `include/nlohmann/` | JSON 解析和序列化。 |
| `include/cppjieba/` | 中文分词。 |
| `include/simhash/` | Simhash 去重。 |
| `include/pybind11/` | C++ 嵌入 Python。 |
| `include/tinyxml2.h`, `src/tinyxml2.cpp` | XML 解析。 |
| `include/Error_check.h` | 简单错误检查宏。 |
| `data/html/` | 客户端结果展示页面。 |

## 18. 完整离线运行流程

离线流程用于准备在线搜索需要的数据文件。

### 18.1 准备语料

已有语料位于：

- `raw_data/module1/yuliao/`
- `raw_data/module2/people_yuliao/`

如果需要重新抓取 RSS/XML：

```bash
cd build
./crawl
```

注意：爬虫模块当前存在硬编码路径，实际运行前需要确认路径可用。

### 18.2 构建关键词推荐词典

```bash
cd build
./dicProducer
```

预期生成或更新：

- `data/dic_data/dicEn.dat`
- `data/dic_data/dicCn.dat`
- `data/dic_data/dicindexEn.dat`
- `data/dic_data/dicindexCn.dat`
- `data/dic_data/dic.dat`
- `data/dic_data/dicIndex.dat`

这些文件供 `Dictionary` 和 `KeyRecommander` 使用。

### 18.3 生成网页库并去重

```bash
cd build
./PageProducer_test
```

预期生成或更新：

- `data/page_data/ripepage.dat`
- `data/page_data/pageoffset.dat`
- `data/page_data/newripepage.dat`
- `data/page_data/newoffset.dat`

其中：

- `ripepage.dat` 是原始网页库。
- `pageoffset.dat` 是原始网页库 offset 表。
- `newripepage.dat` 是去重后的网页库。
- `newoffset.dat` 是去重后网页库 offset 表。

### 18.4 构建网页倒排索引

```bash
cd build
./pageLibPreprocessor_test
```

预期生成或更新：

- `data/page_data/invertIndexTable.dat`

该文件供 `WebPageQuery` 在线检索使用。

## 19. 完整在线运行流程

以下以 `RSserver_Cache2` 为例说明。

### 19.1 启动服务端

```bash
cd build
./RSserver_Cache2
```

服务端启动流程：

1. `main()` 创建 `Configuration`。
2. 构造 `RecommandSearchServer`。
3. 构造线程池、TCP 服务器、缓存管理器。
4. 启动线程池。
5. 注册连接、消息、关闭回调。
6. 启动 TCP server。
7. `Acceptor` 绑定 `config.json` 中的 `ip:port`。
8. `EventLoop` 进入 epoll 循环。

### 19.2 启动客户端

```bash
cd build
./client
```

客户端启动流程：

1. 读取配置。
2. 连接服务器。
3. 等待用户输入查询词。

### 19.3 关键词推荐请求流程

1. 用户输入原始查询词。
2. 客户端构造 JSON：

   ```json
   {
     "id": 0,
     "length": 12,
     "data": "用户输入"
   }
   ```

3. 客户端发送 JSON 文本加换行。
4. 服务端 `EventLoop` 检测到连接 fd 可读。
5. `TcpConnection::recvMsg()` 使用 `SocketIO::readLine()` 读取整行。
6. `ProtocolParser` 将 JSON 解析为 `Message`。
7. 服务端创建 `Mytask` 并加入线程池。
8. 工作线程执行 `Mytask::process()`。
9. 判断 `msg.id == KEY_RECOMMAND`。
10. 使用当前工作线程 ID 获取对应缓存。
11. 先查本地 LRUCache。
12. 缓存未命中时调用 `KeyRecommander::doQuery()`。
13. `KeyRecommander` 查询字索引、计算编辑距离、排序。
14. 生成推荐词 JSON 数组。
15. 写入当前线程缓存。
16. 调用 `TcpConnection::sendInLoop()`。
17. `EventLoop` 被 eventfd 唤醒。
18. IO 线程执行发送。
19. 客户端收到推荐词并打印。

### 19.4 网页搜索请求流程

1. 用户选择或输入搜索词。
2. 客户端构造 JSON：

   ```json
   {
     "id": 1,
     "length": 12,
     "data": "搜索词"
   }
   ```

3. 服务端收到请求并创建任务。
4. 工作线程判断 `msg.id == WEBPAGE_SEARCH`。
5. 调用 `WebPageSearch::doQuery()`。
6. `WebPageQuery` 对搜索词分词。
7. 查询倒排索引。
8. 构造查询向量和文档向量。
9. 计算余弦相似度。
10. 取前 `queryWebPageNum` 个文档。
11. 根据 offset 从 `newripepage.dat` 读取文档。
12. 构造 `WebPage` 对象。
13. 将结果转换为 JSON：

   ```json
   [
     {
       "title": "...",
       "link": "..."
     }
   ]
   ```

14. 通过 `sendInLoop()` 返回客户端。
15. 客户端生成 HTML 文件并打开浏览器展示。

### 19.5 缓存定时同步流程

1. 线程池启动时额外启动 `TimerThread`。
2. `TimerThread` 运行 `TimerManager::start()`。
3. timerfd 周期性触发。
4. `CacheUpdateTask` 调用 `CacheManager::PeriodicalUpdateCache()`。
5. `CacheManager` 遍历每个工作线程缓存。
6. 合并各线程的 pending update 到主缓存。
7. 用主缓存刷新所有工作线程缓存。
8. 到达写盘时间后将主缓存写入 `cacheData`。

## 20. 数据文件关系

### 20.1 关键词推荐数据链

```text
raw_data/module1/yuliao/english.txt
raw_data/module1/yuliao/art/
        |
        v
DicProducer
        |
        v
data/dic_data/dicEn.dat
data/dic_data/dicCn.dat
data/dic_data/dicindexEn.dat
data/dic_data/dicindexCn.dat
        |
        v
data/dic_data/dic.dat
data/dic_data/dicIndex.dat
        |
        v
Dictionary
        |
        v
KeyRecommander
```

### 20.2 网页搜索数据链

```text
raw_data/module2/people_yuliao/
        |
        v
PageProducer
        |
        v
data/page_data/ripepage.dat
data/page_data/pageoffset.dat
        |
        v
Simhash 去重
        |
        v
data/page_data/newripepage.dat
data/page_data/newoffset.dat
        |
        v
pageLibPreprocessor
        |
        v
data/page_data/invertIndexTable.dat
        |
        v
WebPageQuery
        |
        v
WebPageSearch
```

### 20.3 在线服务数据链

```text
client
  |
  | JSON over TCP
  v
TcpServer / EventLoop
  |
  v
ThreadPool
  |
  +--> KeyRecommander --> Dictionary --> dic.dat / dicIndex.dat
  |
  +--> WebPageSearch --> invertIndexTable.dat / newoffset.dat / newripepage.dat
  |
  v
LRUCache / CacheManager / Redis
  |
  v
EventLoop sendInLoop
  |
  v
client
```

## 21. 当前实现中的重要注意点

这些不是项目运行流程的一部分，但重新接手时很有帮助。

1. 运行目录依赖 `build/`
   - `Configuration` 中配置路径是 `../config/config.json`。
   - 推荐从 `build/` 目录运行可执行文件。

2. `src/pageLibPreprocessor.cc` 是历史实现
   - 当前 CMake 使用 `src/Optimize_pageLibPreprocessor.cc`。
   - 旧文件接口与当前头文件不完全匹配。

3. 三个服务端头文件 include guard 相同
   - `RecommandSearchServer_MyCache.h`
   - `RecommandSearchServer_MyCache2.h`
   - `RecommandSearchServer_Myredis.h`
   - 它们都使用 `_RECOMMAND_SEARCH_SERVER_H`，同一翻译单元中不要同时 include。

4. 在线协议当前不是严格长度帧
   - 客户端请求用换行分隔 JSON。
   - 服务端按行读取。
   - 服务端响应没有追加换行或长度头。
   - `TLVMessage` 已实现但未接入主流程。

5. Redis 版本依赖外部服务
   - 运行 `RSserver_Myredis` 前需要 Redis 可连接。
   - CMake 还需要找到 `hiredis` 和 `redis++`。

6. 部分路径硬编码
   - 爬虫模块存在绝对路径。
   - 客户端打开浏览器时也使用了绝对路径和 `google-chrome` 命令。

## 22. 推荐阅读顺序

如果很久没看项目，建议按下面顺序重新熟悉：

1. `config/config.json`
2. `src/Configuration.cc`
3. `include/RecommandSearchServer_MyCache2.h`
4. `src/Reactor/TestRSserver_MyCache2.cc`
5. `src/Reactor/EventLoop.cc`
6. `src/Reactor/ThreadPool.cc`
7. `src/KeyRecommander.cc`
8. `src/WebPageQuery.cc`
9. `src/LRUCache.cc`
10. `src/CacheManager.cc`
11. `src/PageProducer.cc`
12. `src/Optimize_pageLibPreprocessor.cc`
13. `src/DicProducer.cc`

这样可以先建立在线主流程，再回头理解离线数据如何生成。

