# AI 查询模块流程和技术文档

## 1. 模块定位

AI 查询模块是在原有搜索引擎基础上扩展的 RAG 检索增强生成能力。

原有 `WEBPAGE_SEARCH` 负责返回相关网页标题和链接；新增 `AI_SEARCH` 会先复用网页检索结果，再将相关网页内容摘要作为上下文发送给本地或云端大模型，最终返回 AI 答案、来源链接和相关追问。

当前 AI 查询链路支持：

- 客户端通过 `/ai question` 触发 AI 查询。
- 服务端通过 `Protocol::AI_SEARCH` 区分 AI 请求。
- 复用原有倒排索引和网页检索逻辑进行资料召回。
- 使用 `HybridRanker` 对召回网页进行轻量重排。
- 使用 `AIClient` 调用 OpenAI-compatible `/v1/chat/completions` 接口。
- 支持本地 `llama.cpp` 模型服务，也可以通过配置切换为云端大模型。
- 模型调用失败或超时时，自动回退到抽取式答案，保证服务可用性。

## 2. 相关文件

| 文件 | 作用 |
| --- | --- |
| `include/ProtocolParser.h` | 定义 `AI_SEARCH` 协议类型。 |
| `src/clientTest.cc` | 客户端解析 `/ai` 命令并发送 AI 查询请求。 |
| `include/RecommandSearchServer_MyCache2.h` | 服务端任务分发，新增 `AI_SEARCH` 分支。 |
| `include/AISearcher.h` / `src/AISearcher.cc` | AI 查询业务编排层，负责网页召回、重排和答案生成。 |
| `include/HybridRanker.h` / `src/HybridRanker.cc` | 对网页搜索结果进行轻量重排。 |
| `include/AIClient.h` / `src/AIClient.cc` | 构造 prompt、调用大模型、解析响应、生成 fallback 答案。 |
| `include/WebPage.h` / `src/WebPage.cc` | 新增 `getContent()`，为 AI prompt 提供网页正文。 |
| `config/config.json` | 配置 AI 召回数量、模型地址、token 数、超时时间等参数。 |
| `CMakeLists.txt` | 编译 AI 模块，并在找到 libcurl 时启用 HTTPS/HTTP 模型调用能力。 |

## 3. 请求入口

客户端中通过 `/ai` 前缀触发 AI 查询。

示例输入：

```text
/ai 美国民众
```

客户端处理流程：

```text
读取用户输入
-> 判断是否以 "/ai " 开头
-> 设置 msg.id = Protocol::AI_SEARCH
-> msg.data 保存用户问题
-> JSON 序列化
-> 发送给服务端
```

对应代码位置：

- `src/clientTest.cc`
  - `/ai` 命令识别
  - `msg.id = Protocol::AI_SEARCH`
  - `PrintAIAnswer()` 打印 AI 返回结果

## 4. 服务端分发流程

服务端收到请求后，在 `Mytask::process()` 中根据协议 ID 分发任务。

```text
KEY_RECOMMAND   -> KeyRecommander
WEBPAGE_SEARCH  -> WebPageSearch
AI_SEARCH       -> AISearcher
```

AI 查询对应逻辑：

```cpp
else if (m_msg.id == Protocol::AI_SEARCH)
{
    AISearcher aiSearcher(m_msg.data, m_config);
    m_msg.data = aiSearcher.doQuery();
}
```

该设计保持了原有任务分发结构不变，只是在业务层新增一个 AI 查询分支。

## 5. 总体调用时序

```text
Client
  |
  | /ai question
  v
TcpServer / EventLoop
  |
  v
ThreadPool -> Mytask::process()
  |
  v
AISearcher::doQuery()
  |
  |-- WebPageSearch::doQuery()
  |     |
  |     |-- cppjieba 分词
  |     |-- 查询倒排索引
  |     |-- TF-IDF / cosine 排序
  |     |-- 读取网页库并生成 WebPage
  |
  |-- HybridRanker::rank()
  |     |
  |     |-- 标题命中加权
  |     |-- 正文命中加权
  |     |-- 选取 top N 来源
  |
  |-- AIClient::generateAnswer()
        |
        |-- buildPrompt()
        |-- callLargeModel()
        |     |
        |     |-- POST /v1/chat/completions
        |     |-- llama.cpp / OpenAI-compatible API
        |
        |-- parseModelAnswer()
        |-- fallback: buildExtractiveAnswer()
        |-- 组装 JSON 响应
```

## 6. AISearcher 业务编排

`AISearcher` 是 AI 查询的业务编排层，不直接处理网络和模型细节。

核心流程：

1. 接收用户问题。
2. 调用 `WebPageSearch` 进行传统网页召回。
3. 根据 `aiResultNum` 控制候选网页数量。
4. 调用 `HybridRanker` 对网页进行轻量重排。
5. 调用 `AIClient` 生成最终 JSON 响应。

核心代码：

```cpp
WebPageSearch webPageSearch(m_query, m_config);
std::vector<WebPage> pages = webPageSearch.doQuery();

HybridRanker ranker;
std::vector<WebPage> rankedPages =
    ranker.rank(m_query, pages, std::min(limit, pages.size()));

AIClient client(m_config);
std::string result = client.generateAnswer(m_query, rankedPages);
```

## 7. HybridRanker 重排策略

`HybridRanker` 当前实现的是轻量规则重排，不依赖额外模型。

评分逻辑：

```text
标题中出现 query：权重 5.0
正文中出现 query：权重 1.5
标题非空：加 0.5
正文非空：加 0.2
```

这个模块的作用是：在原有 TF-IDF 召回结果基础上，优先选择更适合作为 AI 上下文的网页。

后续可以扩展为：

- BM25 分数融合
- 向量相似度融合
- 标题、正文、发布时间多因子排序
- RRF 混合排序

## 8. AIClient 模型调用

`AIClient` 是 AI 查询模块的核心适配层，主要负责：

1. 构造大模型 prompt。
2. 调用 OpenAI-compatible API。
3. 解析模型响应。
4. 生成统一 JSON 返回结果。
5. 在模型失败时提供 fallback。

### 8.1 Prompt 构造

Prompt 由三部分组成：

```text
用户问题
检索资料
回答要求
```

当前为了优化本地模型速度，prompt 会受到配置限制：

- `llmPromptSourceNum`：最多放入多少条来源。
- `llmSnippetMaxBytes`：每条来源正文摘要最大长度。
- prompt 中不放 URL，URL 只在最终 `sources` 中返回。

示例 prompt 结构：

```text
用户问题：
美国民众

检索资料：
[1] 标题：xxx
内容摘要：xxx

[2] 标题：xxx
内容摘要：xxx

请用不超过三点回答。要求：直接回答问题；每点尽量短；依据后标注来源编号；资料不足时说明不足。
```

### 8.2 模型接口

当前使用 OpenAI-compatible Chat Completions 接口：

```text
POST {llmBaseUrl}/chat/completions
```

请求体示例：

```json
{
  "model": "local-model",
  "temperature": 0.2,
  "max_tokens": 128,
  "messages": [
    {
      "role": "system",
      "content": "你是搜索引擎的AI回答模块..."
    },
    {
      "role": "user",
      "content": "用户问题和检索资料..."
    }
  ]
}
```

### 8.3 libcurl 和本地 HTTP fallback

当编译环境找到 libcurl 时，`CMakeLists.txt` 会定义：

```text
HAVE_CURL
```

此时 `AIClient` 使用 libcurl 发送 HTTP/HTTPS 请求，支持本地 llama.cpp 和云端大模型。

如果没有 libcurl，则使用内置的简易 HTTP 客户端 `plainHttpPost()`。该 fallback 只适合本地 HTTP 服务，例如：

```text
http://127.0.0.1:8080/v1
```

不适合 HTTPS 云端接口。

### 8.4 响应解析

模型响应优先解析：

```text
choices[0].message.content
```

同时兼容：

```text
choices[0].text
output_text
```

如果解析失败，返回空字符串并触发 fallback。

## 9. 返回格式

AI 查询最终返回 JSON 字符串。

示例：

```json
{
  "type": "ai_search",
  "query": "美国民众",
  "answer": "资料中未提供关于美国民众的具体信息。[1][2]",
  "llm_used": true,
  "llm_model": "local-model",
  "sources": [
    {
      "index": 1,
      "title": "巴以冲突升级对中东局势影响几何",
      "link": "http://world.people.com.cn/...",
      "snippet": "..."
    }
  ],
  "related_queries": [
    "美国民众 最新进展",
    "美国民众 原因分析",
    "美国民众 影响"
  ]
}
```

字段说明：

| 字段 | 说明 |
| --- | --- |
| `type` | 响应类型，固定为 `ai_search`。 |
| `query` | 用户原始问题。 |
| `answer` | 大模型生成答案或 fallback 答案。 |
| `llm_used` | 是否成功使用大模型。 |
| `llm_model` | 使用的模型名称配置。 |
| `sources` | 检索来源列表。 |
| `related_queries` | 相关追问建议。 |

## 10. 配置项

AI 查询相关配置集中在 `config/config.json`。

| 配置项 | 当前值 | 作用 |
| --- | --- | --- |
| `aiResultNum` | `2` | AI 查询进入模型前保留的来源数量。 |
| `llmPromptSourceNum` | `2` | prompt 中实际放入的来源数量。 |
| `llmSnippetMaxBytes` | `160` | 每条来源摘要最大字节数。 |
| `llmBaseUrl` | `http://127.0.0.1:8080/v1` | OpenAI-compatible API 地址。 |
| `llmApiKey` | `no-key` | API Key，本地 llama.cpp 可使用占位值。 |
| `llmApiKeyEnv` | `""` | 从环境变量读取 API Key，优先级高于 `llmApiKey`。 |
| `llmModel` | `local-model` | 模型名称。 |
| `llmMaxTokens` | `128` | 模型最大输出 token 数。 |
| `llmTemperature` | `0.2` | 模型生成随机性。 |
| `llmTimeoutMs` | `45000` | 模型调用超时时间。 |

## 11. 本地模型服务

本地 llama.cpp 启动示例：

```bash
./llama-server -m /path/to/model.gguf \
  --host 127.0.0.1 \
  --port 8080 \
  -c 2048 \
  -t 8
```

如果有 NVIDIA GPU，可以增加：

```bash
-ngl 99
```

服务启动后，项目中的 `llmBaseUrl` 保持：

```json
"llmBaseUrl" : "http://127.0.0.1:8080/v1"
```

## 12. 性能分析

AI 查询比普通网页搜索慢，原因是多了大模型推理阶段。

普通网页搜索：

```text
分词 -> 查倒排索引 -> 排序 -> 返回 title/link
```

AI 查询：

```text
分词 -> 查倒排索引 -> 排序 -> 构造 prompt -> 模型推理 -> 返回答案
```

从 llama.cpp 日志看，主要耗时通常分为：

```text
prompt eval time：模型读取 prompt 的时间
eval time：模型生成答案的时间
total time：总耗时
```

当前项目已做的优化：

- 限制来源数量：`aiResultNum`
- 限制 prompt 来源数：`llmPromptSourceNum`
- 限制摘要长度：`llmSnippetMaxBytes`
- 限制输出长度：`llmMaxTokens`
- prompt 中去掉 URL，减少无效 token
- 模型失败时 fallback，避免请求长期卡死

继续优化方向：

- 使用更小模型，例如 3B/4B Instruct GGUF。
- 使用 Q4_K_M 量化模型。
- GPU offload：`-ngl 99`。
- 做流式输出，让客户端边生成边显示。
- 做 AI 结果缓存，相同 query 直接返回。
- 改成异步页面，先展示 sources，再更新 AI 答案。

## 13. 异常和 fallback

以下情况会触发 fallback：

- `llmBaseUrl` 为空。
- 连接模型服务失败。
- HTTP 状态码不是 2xx。
- libcurl 请求超时。
- 模型响应 JSON 解析失败。
- 响应中没有可提取的 answer 字段。

fallback 答案由 `buildExtractiveAnswer()` 生成，基于本地检索结果抽取标题和摘要，不依赖模型。

该设计保证：即使 llama.cpp 未启动，AI 查询也能返回可读结果，而不是直接失败。

## 14. 当前模块边界

AI 查询模块目前不改变原有搜索引擎核心索引结构。

它的边界是：

```text
传统检索负责召回资料
AI 模块负责阅读资料并生成答案
```

这种设计的优点：

- 不破坏原有关键词推荐和网页搜索逻辑。
- AI 模块可以独立替换模型服务。
- 支持本地 llama.cpp 和云端 OpenAI-compatible API。
- 检索结果仍保留来源链接，答案可追溯。

当前限制：

- 召回仍主要依赖关键词和 TF-IDF，不是真正语义向量召回。
- 模型回答质量依赖召回资料质量。
- 当前协议是一次请求一次完整响应，不支持流式输出。
- 本地 CPU 推理存在固定延迟。

## 15. 后续扩展计划

### 15.1 AI 结果缓存

对相同 query 的 AI 响应进行缓存：

```text
query -> answer JSON
```

命中缓存时可绕过模型调用，响应速度接近普通网页搜索。

### 15.2 流式输出

让 llama.cpp 使用 stream 模式，服务端边接收 token 边推给客户端，降低用户感知延迟。

### 15.3 异步 AI 页面

客户端先展示网页 sources，再异步等待 AI 答案：

```text
立即返回 sources 页面
页面显示 AI 正在生成
答案生成后更新页面
```

### 15.4 语义向量召回

离线为网页 chunk 生成 embedding，在线阶段进行向量召回：

```text
关键词召回 + 向量召回 + 混合重排 + LLM 答案生成
```

该方案可以提升自然语言问题的召回质量。

## 16. 总结

AI 查询模块将原有传统搜索引擎扩展为基础版 RAG 系统。

它的核心价值不是替代原来的网页检索，而是在原有检索能力上增加一层“阅读和总结”能力：

```text
原网页检索：返回相关网页
AI 查询：基于相关网页生成带来源的答案
```

当前实现已经具备完整闭环：

```text
客户端 /ai 输入
-> 服务端 AI_SEARCH 分发
-> 网页召回
-> 轻量重排
-> prompt 构造
-> llama.cpp / OpenAI-compatible 模型调用
-> JSON 答案返回
-> 客户端展示 answer、sources、related queries
```

后续优化重点主要集中在速度、召回质量和交互体验三个方向。
