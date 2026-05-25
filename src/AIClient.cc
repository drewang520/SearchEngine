#include "AIClient.h"
#include "ProtocolParser.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cstdlib>
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#include <cstring>
#include <netdb.h>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

using Protocol::ProtocolParser;
using json = nlohmann::json;

namespace
{
std::string compactWhitespace(std::string text)
{
    for (char& ch : text)
    {
        if (ch == '\n' || ch == '\r' || ch == '\t')
        {
            ch = ' ';
        }
    }

    auto newEnd = std::unique(text.begin(), text.end(),
        [](char lhs, char rhs) {
            return lhs == ' ' && rhs == ' ';
        });
    text.erase(newEnd, text.end());
    return text;
}

std::string truncateUtf8(const std::string& text, std::size_t maxBytes)
{
    if (text.size() <= maxBytes)
    {
        return text;
    }

    std::size_t end = maxBytes;
    while (end > 0 && (static_cast<unsigned char>(text[end]) & 0xC0) == 0x80)
    {
        --end;
    }
    return text.substr(0, end) + "...";
}

std::size_t writeCallback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
{
    auto* response = static_cast<std::string*>(userdata);
    const std::size_t realSize = size * nmemb;
    response->append(ptr, realSize);
    return realSize;
}

std::string joinUrl(std::string baseUrl, const std::string& path)
{
    while (!baseUrl.empty() && baseUrl.back() == '/')
    {
        baseUrl.pop_back();
    }
    return baseUrl + path;
}

void initCurlOnce()
{
#ifdef HAVE_CURL
    static std::once_flag flag;
    std::call_once(flag, []() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    });
#endif
}

struct ParsedHttpUrl
{
    std::string host;
    std::string port;
    std::string path;
};

bool parseHttpUrl(const std::string& url, ParsedHttpUrl& parsed)
{
    const std::string prefix = "http://";
    if (url.rfind(prefix, 0) != 0)
    {
        return false;
    }

    std::size_t hostStart = prefix.size();
    std::size_t pathStart = url.find('/', hostStart);
    std::string hostPort = pathStart == std::string::npos
        ? url.substr(hostStart)
        : url.substr(hostStart, pathStart - hostStart);

    parsed.path = pathStart == std::string::npos ? "/" : url.substr(pathStart);
    std::size_t colon = hostPort.rfind(':');
    if (colon == std::string::npos)
    {
        parsed.host = hostPort;
        parsed.port = "80";
    }
    else
    {
        parsed.host = hostPort.substr(0, colon);
        parsed.port = hostPort.substr(colon + 1);
    }
    return !parsed.host.empty() && !parsed.port.empty();
}

bool sendAll(int fd, const std::string& data)
{
    const char* ptr = data.data();
    std::size_t left = data.size();
    while (left > 0)
    {
        ssize_t sent = ::send(fd, ptr, left, 0);
        if (sent <= 0)
        {
            return false;
        }
        ptr += sent;
        left -= static_cast<std::size_t>(sent);
    }
    return true;
}

std::string plainHttpPost(const std::string& url,
                          const std::string& requestBody,
                          const std::string& apiKey,
                          long timeoutSeconds)
{
    ParsedHttpUrl parsed;
    if (!parseHttpUrl(url, parsed))
    {
        return {};
    }

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    if (getaddrinfo(parsed.host.c_str(), parsed.port.c_str(), &hints, &result) != 0)
    {
        return {};
    }

    int fd = -1;
    for (addrinfo* rp = result; rp != nullptr; rp = rp->ai_next)
    {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == -1)
        {
            continue;
        }
        timeval timeout{};
        timeout.tv_sec = timeoutSeconds;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(result);

    if (fd == -1)
    {
        return {};
    }

    std::ostringstream oss;
    oss << "POST " << parsed.path << " HTTP/1.1\r\n"
        << "Host: " << parsed.host << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << requestBody.size() << "\r\n"
        << "Connection: close\r\n";
    if (!apiKey.empty())
    {
        oss << "Authorization: Bearer " << apiKey << "\r\n";
    }
    oss << "\r\n" << requestBody;

    if (!sendAll(fd, oss.str()))
    {
        close(fd);
        return {};
    }

    std::string rawResponse;
    char buffer[4096];
    while (true)
    {
        ssize_t nread = recv(fd, buffer, sizeof(buffer), 0);
        if (nread > 0)
        {
            rawResponse.append(buffer, static_cast<std::size_t>(nread));
            continue;
        }
        break;
    }
    close(fd);

    std::size_t statusEnd = rawResponse.find("\r\n");
    if (statusEnd == std::string::npos)
    {
        return {};
    }

    std::string statusLine = rawResponse.substr(0, statusEnd);
    if (statusLine.find(" 2") == std::string::npos)
    {
        return {};
    }

    std::size_t bodyStart = rawResponse.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
    {
        return {};
    }
    return rawResponse.substr(bodyStart + 4);
}
}

AIClient::AIClient(const Configuration& config)
: m_config(config)
{
}

std::string AIClient::generateAnswer(const std::string& query,
                                     const std::vector<WebPage>& pages) const
{
    json response;
    response["type"] = "ai_search";
    response["query"] = query;

    if (pages.empty())
    {
        response["answer"] = "没有在本地网页库中检索到足够相关的资料，建议换一个更具体的关键词再试。";
        response["sources"] = json::array();
        response["related_queries"] = makeRelatedQueries(query);
        return ProtocolParser::JsonToString(response);
    }

    std::string answer = callLargeModel(query, pages);
    bool usedLargeModel = true;
    if (answer.empty())
    {
        answer = buildExtractiveAnswer(query, pages);
        usedLargeModel = false;
    }
    response["answer"] = answer;
    response["llm_used"] = usedLargeModel;
    response["llm_model"] = configValue("llmModel", "local-model");

    json sources = json::array();
    for (std::size_t idx = 0; idx < pages.size(); ++idx)
    {
        json item;
        item["index"] = idx + 1;
        item["title"] = pages[idx].getTitle();
        item["link"] = pages[idx].getLink();
        item["snippet"] = makeSnippet(pages[idx], 180);
        sources.push_back(item);
    }
    response["sources"] = sources;
    response["related_queries"] = makeRelatedQueries(query);
    return ProtocolParser::JsonToString(response);
}

std::string AIClient::callLargeModel(const std::string& query,
                                     const std::vector<WebPage>& pages) const
{
    const std::string baseUrl = configValue("llmBaseUrl", "");
    if (baseUrl.empty())
    {
        return {};
    }

    initCurlOnce();

    const std::string apiKeyEnv = configValue("llmApiKeyEnv", "");
    std::string apiKey = configValue("llmApiKey", "");
    if (!apiKeyEnv.empty())
    {
        const char* value = std::getenv(apiKeyEnv.c_str());
        if (value != nullptr)
        {
            apiKey = value;
        }
    }

    json request;
    request["model"] = configValue("llmModel", "local-model");
    request["temperature"] = std::stod(configValue("llmTemperature", "0.2"));
    request["max_tokens"] = std::stoi(configValue("llmMaxTokens", "512"));
    request["messages"] = json::array({
        {
            {"role", "system"},
            {"content", "你是搜索引擎的AI回答模块。只能基于给定资料回答，不要编造资料之外的信息。回答必须使用中文，并在关键结论后标注来源编号，例如[1]。"}
        },
        {
            {"role", "user"},
            {"content", buildPrompt(query, pages)}
        }
    });

    std::string responseBody;
    std::string requestBody = request.dump();
    const std::string url = joinUrl(baseUrl, "/chat/completions");

#ifdef HAVE_CURL
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        return {};
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader;
    if (!apiKey.empty())
    {
        authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestBody.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, std::stol(configValue("llmTimeoutMs", "30000")));

    const CURLcode code = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (code != CURLE_OK || httpCode < 200 || httpCode >= 300)
    {
        return {};
    }

    return parseModelAnswer(responseBody);
#else
    long timeoutMs = std::stol(configValue("llmTimeoutMs", "30000"));
    responseBody = plainHttpPost(url, requestBody, apiKey, std::max<long>(1, timeoutMs / 1000));
    if (responseBody.empty())
    {
        return {};
    }
    return parseModelAnswer(responseBody);
#endif
}

std::string AIClient::buildPrompt(const std::string& query,
                                  const std::vector<WebPage>& pages) const
{
    std::string prompt = "用户问题：\n" + query + "\n\n检索资料：\n";
    const std::size_t sourceLimit = std::min(configSizeValue("llmPromptSourceNum", 3), pages.size());
    const std::size_t snippetMaxBytes = configSizeValue("llmSnippetMaxBytes", 240);
    for (std::size_t idx = 0; idx < sourceLimit; ++idx)
    {
        prompt += "[" + std::to_string(idx + 1) + "] 标题：" + pages[idx].getTitle() + "\n";
        prompt += "内容摘要：" + makeSnippet(pages[idx], snippetMaxBytes) + "\n\n";
    }
    prompt += "请用不超过三点回答。要求：直接回答问题；每点尽量短；依据后标注来源编号；资料不足时说明不足。";
    return prompt;
}

std::string AIClient::parseModelAnswer(const std::string& responseBody) const
{
    try
    {
        json response = json::parse(responseBody);
        if (response.contains("choices")
            && response["choices"].is_array()
            && !response["choices"].empty())
        {
            const auto& choice = response["choices"][0];
            if (choice.contains("message")
                && choice["message"].contains("content")
                && choice["message"]["content"].is_string())
            {
                return choice["message"]["content"].get<std::string>();
            }
            if (choice.contains("text") && choice["text"].is_string())
            {
                return choice["text"].get<std::string>();
            }
        }

        if (response.contains("output_text") && response["output_text"].is_string())
        {
            return response["output_text"].get<std::string>();
        }
    }
    catch (const std::exception&)
    {
        return {};
    }
    return {};
}

std::string AIClient::configValue(const std::string& key,
                                  const std::string& defaultValue) const
{
    auto it = m_config.getConfig().find(key);
    if (it == m_config.getConfig().end())
    {
        return defaultValue;
    }
    return it->second;
}

std::size_t AIClient::configSizeValue(const std::string& key,
                                      std::size_t defaultValue) const
{
    auto it = m_config.getConfig().find(key);
    if (it == m_config.getConfig().end())
    {
        return defaultValue;
    }
    return static_cast<std::size_t>(std::stoul(it->second));
}

std::string AIClient::buildExtractiveAnswer(const std::string& query,
                                            const std::vector<WebPage>& pages) const
{
    std::string answer = "基于本地网页库的检索结果，关于“" + query + "”可以先关注这些信息：";
    const std::size_t limit = std::min<std::size_t>(pages.size(), 3);
    for (std::size_t idx = 0; idx < limit; ++idx)
    {
        answer += " [" + std::to_string(idx + 1) + "] ";
        answer += pages[idx].getTitle();
        const std::string snippet = makeSnippet(pages[idx], 120);
        if (!snippet.empty())
        {
            answer += "：" + snippet;
        }
    }
    answer += " 以上内容由检索结果抽取生成，建议结合来源链接继续核对细节。";
    return answer;
}

std::string AIClient::makeSnippet(const WebPage& page, std::size_t maxLen) const
{
    std::string content = compactWhitespace(page.getContent());
    if (content.empty())
    {
        content = compactWhitespace(page.getTitle());
    }
    return truncateUtf8(content, maxLen);
}

std::vector<std::string> AIClient::makeRelatedQueries(const std::string& query) const
{
    return {
        query + " 最新进展",
        query + " 原因分析",
        query + " 影响"
    };
}
