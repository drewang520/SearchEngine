#ifndef _HYBRID_RANKER_H
#define _HYBRID_RANKER_H

#include "WebPage.h"
#include <cstddef>
#include <string>
#include <vector>

class HybridRanker
{
public:
    std::vector<WebPage> rank(const std::string& query,
                              const std::vector<WebPage>& pages,
                              std::size_t limit) const;

private:
    double scorePage(const std::string& query, const WebPage& page) const;
};

#endif
