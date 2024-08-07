#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "ThreadPool.h"

using namespace std;

class Client {
    string szIp;
    int iPort;
    int iSockFd;

public:
    Client(){}
    Client(int iInSockFd, const struct sockaddr_in& info) {
        char ip[16] = "";
        sprintf(ip, "%s", inet_ntoa(info.sin_addr));
        szIp = string(ip);
        iPort = info.sin_port;
        iSockFd = iInSockFd;
    }

    void show() {
        cout << "client ip =  " << szIp << endl;
        cout << "client port =  " << iPort << endl;
        cout << "client Sock fd =  " << iSockFd << endl;
    }
};

// 对文件描述符设置非阻塞
int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode) {
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode) // ET
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event); 
    // 设置非阻塞
    setnonblocking(fd);
}

class FirstServer {
    int m_iPort;
    int m_iListenfd;

    int m_iEpollfd;

    ThreadPool m_thdPool;

public:
    FirstServer();
    ~FirstServer();

    void eventListen();
    void run();

    // 客户端连接
    void dealclient();

    // 读客户端数据
    void dealwithread(int iCliFd);

    // 发送数据
    void dealwithwrite();
};

FirstServer::FirstServer() :m_thdPool(3) {
    m_iPort = 8888;
    m_iListenfd = 0;
    m_iEpollfd = 0;
}

FirstServer::~FirstServer() {
    close(m_iListenfd);
}

void FirstServer::eventListen() {
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
    // 防止在短时间关闭服务端启动时无法启动
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
    // LT
    addfd(m_iEpollfd, m_iListenfd, false, 0); // 监听fd不能设置EPOLLONESHOT,否则
    addfd(m_iEpollfd, STDIN_FILENO, false, 0);
}

void FirstServer::run(){
    
    const int MAX_EVENT_NUMBER = 10000;
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
                // 新客户端连接
                cout << "有连接来了" << number << endl;
                auto fAccept = std::bind(&FirstServer::dealclient, this);
                m_thdPool.addTask(fAccept);
            } else if (events[i].events & EPOLLIN) {
                // 读取客户端数据
                auto fRead = std::bind(&FirstServer::dealwithread, this, iSockfd);
                m_thdPool.addTask(fRead);
            } else if (events[i].events & EPOLLOUT) {
                // 写数据
                // dealwithwrite
            }
        }
    }
}

// 客户端连接
void FirstServer::dealclient() {
    struct sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);

    int iCliFd = accept(m_iListenfd, (struct sockaddr*)&clientAddress, &clientAddrLen);
    if (iCliFd < 0) {
        perror("accept error ");
        return;
    }

    cout << "accept success " << endl;
    Client stCli(iCliFd, clientAddress);
    stCli.show();

    addfd(m_iEpollfd, iCliFd, true, 0);
}

// 读客户端数据
void FirstServer::dealwithread(int iCliFd) {
    char msg[1024];
    int iRet = read(iCliFd, msg, 1024);
    if(iRet == 0) {
        cout << "iCliFd  close fd = " << iCliFd << endl;
        close(iCliFd);
    }
    else if(iRet < 0) {
        perror("read : ");
    }
    else if (iRet > 0) {
        printf("%s\n", msg);
    }
}

// 发送数据
void FirstServer::dealwithwrite() {}

int main(int argc, char* argv[]) {
    FirstServer server;
    server.eventListen();

    server.run();
    return 0;
}
