#include "HybridRanker.h"
#include <algorithm>

namespace
{
std::size_t countOccurrences(const std::string& text, const std::string& key)
{
    if (text.empty() || key.empty())
    {
        return 0;
    }

    std::size_t count = 0;
    std::size_t pos = text.find(key);
    while (pos != std::string::npos)
    {
        ++count;
        pos = text.find(key, pos + key.size());
    }
    return count;
}
}

std::vector<WebPage> HybridRanker::rank(const std::string& query,
                                        const std::vector<WebPage>& pages,
                                        std::size_t limit) const
{
    std::vector<std::pair<WebPage, double>> scoredPages;
    scoredPages.reserve(pages.size());

    for (const auto& page : pages)
    {
        scoredPages.push_back({page, scorePage(query, page)});
    }

    std::stable_sort(scoredPages.begin(), scoredPages.end(),
        [](const auto& lhs, const auto& rhs) {
            return lhs.second > rhs.second;
        });

    std::vector<WebPage> result;
    result.reserve(std::min(limit, scoredPages.size()));
    for (std::size_t idx = 0; idx < scoredPages.size() && result.size() < limit; ++idx)
    {
        result.push_back(scoredPages[idx].first);
    }
    return result;
}

double HybridRanker::scorePage(const std::string& query, const WebPage& page) const
{
    double score = 0.0;
    score += static_cast<double>(countOccurrences(page.getTitle(), query)) * 5.0;
    score += static_cast<double>(countOccurrences(page.getContent(), query)) * 1.5;

    if (!page.getTitle().empty())
    {
        score += 0.5;
    }
    if (!page.getContent().empty())
    {
        score += 0.2;
    }
    return score;
}
