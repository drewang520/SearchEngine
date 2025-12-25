#include "KeyRecommander.h"
#include "Configuration.h"

KeyRecommander::KeyRecommander(string queryWords, const TcpConnectionPtr& con, Configuration * config)
: _queryWords(queryWords)
, _conn(con)
, _config(config)
, _pDict(new Dictionary(_config))
{

}

vector<string> KeyRecommander::doQuery()
{

}
