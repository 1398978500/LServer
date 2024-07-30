#include <string>

#include "NanoLog.h"

using namespace std;

int main(int argc, char const* argv[]) {
    initialize(GuaranteedLogger(), "other/", "log", 1);

    LOG_INFO << "log 1";

    string sz = "你好啊";
    LOG_CRIT << sz;

    return 0;
}
