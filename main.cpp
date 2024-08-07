#include <iostream>

#include "LServer.h"
#include "LUtil.h"

using namespace std;

int main(int argc, char* argv[]) {
    int iPort = 8880;
    int iListenMode = TRIG_MODE_ET;
    int iConnMode = TRIG_MODE_LT;
    int iThreadPoolNum = 5;
    int iActMode = MODE_REACTOR;

    LServer server(iPort, iListenMode, iConnMode, iThreadPoolNum, iActMode);

    server.eventListen();

    server.eventLoop();

    return 0;
}
