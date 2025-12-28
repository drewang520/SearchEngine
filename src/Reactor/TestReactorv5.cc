#include "RecommandSearchServer.h"


int main(int argc, char * argv[])
{
    EchoServer echoserver(5, 10, "127.0.0.1", 8888);
    echoserver.start();

    return 0;
}
