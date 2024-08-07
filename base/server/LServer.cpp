#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "LServer.h"
#include "LUtil.h"

using namespace std;

const int DEFAULT_POOL_NUM = 5;
const int DEFAULT_PORT = 8888;
// 最大epoll监听数
const int MAX_EVENT_NUMBER = 10000;

// m_thdPool(DEFAULT_POOL_NUM), m_iPort(DEFAULT_PORT), m_iActMode(MODE_REACTOR)
LServer::LServer() : LServer(DEFAULT_PORT, TRIG_MODE_ET, TRIG_MODE_LT, DEFAULT_POOL_NUM, MODE_REACTOR) {}

LServer::LServer(int iPort, int iListenMode, int iConnMode, int iThreadPoolNum, int iActMode)
    : m_iPort(iPort), m_iListenMode(iListenMode), m_iConnMode(iConnMode), m_thdPool(iThreadPoolNum), m_iActMode(iActMode) {}

LServer::~LServer() {
    close(m_iListenfd);
    close(m_iEpollfd);
}

void LServer::eventListen() {
    // 基本步骤
    m_iListenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_iListenfd < 0) {
        // cerr << "socket 失败了" << m_iListenfd << endl;
        perror("socket error ");
        exit(-1);
    }

#if 0
    // 可以设置SO_LINGER 防止TIME_WAIT 问题：发送失败的数据就不会再发送了
    struct linger tmp = {0, 1}; // {1, 1} 第一个字段表示是否开启，第二个字段表示超时时间
    setsockopt(m_iListenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp))
#endif

    int iRet = 0;

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_iPort);

    int iFlag = 1;
    // SO_REUSEADDR 防止在短时间关闭服务端启动时无法启动
    setsockopt(m_iListenfd, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));

    iRet = bind(m_iListenfd, (struct sockaddr*)&address, sizeof(address));
    if (iRet < 0) {
        perror("bind error ");
        exit(-1);
    }

    iRet = listen(m_iListenfd, 5);
    if (iRet < 0) {
        perror("listen error ");
        exit(-1);
    }

    // epoll
    m_iEpollfd = epoll_create(5);
    if (m_iEpollfd == -1) {
        perror("epoll_create ");
        exit(-1);
    }

    // 监听fd不能设置EPOLLONESHOT
    LUtil::addfd(m_iEpollfd, m_iListenfd, false, m_iListenMode, true);

    LUtil::addfd(m_iEpollfd, STDIN_FILENO, false, TRIG_MODE_LT, true);
}

void LServer::eventLoop() {
    epoll_event events[MAX_EVENT_NUMBER];

    cout << "server start ..." << endl;

    bool bStop = false;
    //
    while (!bStop) {
        int number = epoll_wait(m_iEpollfd, events, MAX_EVENT_NUMBER, -1);
        // EINTR 系统调用被中断了/没写成功数据或没读到数据，需要重新尝试
        if (number < 0 && errno != EINTR) {
            perror("epoll_wait error ");
            break;
        }

        for (int i = 0; i < number; i++) {
            int iSockfd = events[i].data.fd;

            // 监听输入
            if (iSockfd == STDIN_FILENO) {
                char c = 0;
                cin >> c;
                cout << c << endl;
                if (c == 'q') {
                    bStop = true;
                }
            } else if (iSockfd == m_iListenfd) {
                auto fAccept = std::bind(&LServer::dealclient, this);

                cout << "fAccept is nullptr" << endl;

                m_thdPool.addTask(fAccept);
            } else if (events[i].events & EPOLLIN) {
                // 读取客户端数据
                auto fRead = std::bind(&LServer::dealwithread, this, iSockfd);
                m_thdPool.addTask(fRead);
            } else if (events[i].events & EPOLLOUT) {
                // 写数据
                // dealwithwrite
            }
        }
    }
}

// 客户端连接
void LServer::dealclient() {
    struct sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);

    int iCliFd = accept(m_iListenfd, (struct sockaddr*)&clientAddress, &clientAddrLen);
    if (iCliFd < 0) {
        perror("accept error ");
        return;
    }

    LClient stCli(iCliFd, clientAddress);
    stCli.show();

    // 客户端连接设置点对点
    LUtil::addfd(m_iEpollfd, iCliFd, true, m_iConnMode, true);
}

// 读客户端数据
void LServer::dealwithread(int iCliFd) {
    char msg[1024];
    int iRet = read(iCliFd, msg, 1024);
    if (iRet == 0) {
        cout << "iCliFd  close fd = " << iCliFd << endl;
        close(iCliFd);
    } else if (iRet < 0) {
        perror("read : ");
    } else if (iRet > 0) {
        printf("%s\n", msg);
    }
}

// 发送数据
void LServer::dealwithwrite() {}

// 客户端
LClient::LClient(int iInSockFd, const struct sockaddr_in& info) {
    char ip[16] = "";
    sprintf(ip, "%s", inet_ntoa(info.sin_addr));
    szIp = string(ip);
    iPort = info.sin_port;
    iSockFd = iInSockFd;
}

void LClient::show() {
    cout << "client ip =  " << szIp << endl;
    cout << "client port =  " << iPort << endl;
    cout << "client Sock fd =  " << iSockFd << endl;
}