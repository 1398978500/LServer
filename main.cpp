#include <iostream>

#include "LServer.h"
#include "LUtil.h"
#include "NanoLog.h"

using namespace std;

int main(int argc, char* argv[]) {
    initialize(GuaranteedLogger(), "other/", "log", 1);

    int iPort = 8888;
    int iListenMode = TRIG_MODE_ET;
    int iConnMode = TRIG_MODE_LT;
    int iThreadPoolNum = 5;
    int iActMode = MODE_REACTOR;

    LServer server(iPort, iListenMode, iConnMode, iThreadPoolNum, iActMode);

    server.eventLoop();

    return 0;
}
