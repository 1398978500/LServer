#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

class Client {
    string szIp;
    int iPort;
    int iSockFd;

public:
    Client(int iInSockFd, const struct sockaddr_in &info) {
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


class WebServer {
    int m_iPort;
    int m_iListenfd;


public:
    WebServer();
    ~WebServer();

    void eventListen();
};

WebServer::WebServer() {
    m_iPort = 8888;
}

WebServer::~WebServer() {
    close(m_iListenfd);
}

void WebServer::eventListen() {
    // 基本步骤
    m_iListenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(m_iListenfd < 0) {
        // cerr << "socket 失败了" << m_iListenfd << endl;
        perror("socket error ");
        exit(-1);
    }

    int iRet = 0;

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_iPort);

    iRet = bind(m_iListenfd, (struct sockaddr*)&address, sizeof(address));
    if(iRet < 0) {
        perror("bind error ");
        exit(-1);
    }

    iRet = listen(m_iListenfd, 5);
    if(iRet < 0) {
        perror("listen error ");
        exit(-1);
    }

    struct sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);
    cout << "accept ..." << endl;
    int iCliFd = accept(m_iListenfd, (struct sockaddr *)&clientAddress, &clientAddrLen);
    if(iCliFd < 0) {
        perror("accept error ");
        exit(-1);
    }

    cout << "accept success "  << endl;
    Client stCli(iCliFd, clientAddress);
    stCli.show();

    char msg[1024];
    iRet = read(iCliFd, msg, 1024);
    if(iRet > 0) {
        printf("%s\n", msg);

    }

    close(iCliFd);
}

int main(int argc, char* argv[]) {
    WebServer server;
    server.eventListen();

    return 0;
}

