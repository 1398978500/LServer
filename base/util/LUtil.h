#ifndef __LUTIL_H__
#define __LUTIL_H__

enum { MODE_REACTOR = 0, MODE_PROACTOR = 1 };
enum { TRIG_MODE_LT = 0, TRIG_MODE_ET = 1 };

const int MAX_FD = 65535;

class LUtil {
public:
    // 设置文件描述符为非阻塞
    static int setnonblocking(int fd);

    /** 将fd添加到epoll中
     *  iEpollfd : epoll_create创建的epoll文件描述符
     *  iFd      : 要监听的文件描述符
     *  bOneShot : 是否开启EPOLLONESHOT 即同一套接字由一个进程管理 默认开启
     *  iTRIGMode: 触发模式 默认TRIG_MODE_LT
     *  bNonBlock: 是否开启非阻塞 默认不开启
     */
    static bool addfd(int iEpollfd, int iFd, bool bOneShot = true, int iTRIGMode = TRIG_MODE_LT, bool bNonBlock = false);

    static bool delfd(int iEpollfd, int iFd);

    static bool ModFd(int iEpollfd, int iFd, unsigned int uiEvents);
};

#endif
