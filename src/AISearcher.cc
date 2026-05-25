#include "AISearcher.h"
#include "AIClient.h"
#include "HybridRanker.h"
#include "Logger.h"
#include "WebPageSearcher.h"
#include <algorithm>

AISearcher::AISearcher(const std::string& query, const Configuration& config)
: m_query(query)
, m_config(config)
{
}

std::string AISearcher::doQuery()
{
    LOG_INFO("task=AI_SEARCH query=" + m_query);

    WebPageSearch webPageSearch(m_query, m_config);
    std::vector<WebPage> pages = webPageSearch.doQuery();

    std::size_t limit = 5;
    auto it = m_config.getConfig().find("aiResultNum");
    if (it != m_config.getConfig().end())
    {
        limit = static_cast<std::size_t>(std::stoi(it->second));
    }

    HybridRanker ranker;
    std::vector<WebPage> rankedPages = ranker.rank(m_query, pages, std::min(limit, pages.size()));

    AIClient client(m_config);
    std::string result = client.generateAnswer(m_query, rankedPages);
    LOG_DEBUG("AI_SEARCH response bytes=" + std::to_string(result.size()));
    return result;
}
