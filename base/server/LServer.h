#ifndef __LSERVER_H__
#define __LSERVER_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <unordered_map>

#include "LProcess.h"
#include "ThreadPool.h"

class LServer {
    // 端口
    int m_iPort;

    int m_iListenfd;
    int m_iEpollfd;

    ThreadPool m_thdPool;

    // 文件描述符模式 LT/ET
    int m_iListenMode;
    int m_iConnMode;

    // reactor/proactor
    int m_iActMode;

    bool m_bStop;

    std::unordered_map<int, std::shared_ptr<LProcess>> m_mUser;

public:
    LServer();
    LServer(int iPort, int iListenMode, int iConnMode, int iThreadPoolNum, int iActMode);
    ~LServer();

    int eventListen();
    void eventLoop();

    // 客户端连接
    void dealwithaccept();

    // 读客户端数据
    void dealwithread(int iCliFd);

    // 发送数据
    void dealwithwrite(int iCliFd);
};

#endif
