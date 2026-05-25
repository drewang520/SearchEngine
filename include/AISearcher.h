#ifndef _AI_SEARCHER_H
#define _AI_SEARCHER_H

#include "Configuration.h"
#include <string>

class AISearcher
{
public:
    AISearcher(const std::string& query, const Configuration& config);
    std::string doQuery();

private:
    std::string m_query;
    const Configuration& m_config;
};

#endif
