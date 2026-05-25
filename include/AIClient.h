#ifndef _AI_CLIENT_H
#define _AI_CLIENT_H

#include "Configuration.h"
#include "WebPage.h"
#include <string>
#include <vector>

class AIClient
{
public:
    explicit AIClient(const Configuration& config);
    std::string generateAnswer(const std::string& query,
                               const std::vector<WebPage>& pages) const;

private:
    std::string callLargeModel(const std::string& query,
                               const std::vector<WebPage>& pages) const;
    std::string buildPrompt(const std::string& query,
                            const std::vector<WebPage>& pages) const;
    std::string parseModelAnswer(const std::string& responseBody) const;
    std::string configValue(const std::string& key,
                            const std::string& defaultValue) const;
    std::size_t configSizeValue(const std::string& key,
                                std::size_t defaultValue) const;
    std::string buildExtractiveAnswer(const std::string& query,
                                      const std::vector<WebPage>& pages) const;
    std::string makeSnippet(const WebPage& page, std::size_t maxLen) const;
    std::vector<std::string> makeRelatedQueries(const std::string& query) const;

private:
    const Configuration& m_config;
};

#endif
