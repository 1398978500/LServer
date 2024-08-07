#include <fcntl.h>
#include <sys/epoll.h>

#include "LUtil.h"

// 设置文件描述符为非阻塞
int LUtil::setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/** 将fd添加到epoll中
 *  iEpollfd : epoll_create创建的epoll文件描述符
 *  iFd      : 要监听的文件描述符
 *  bOneShot : 是否开启EPOLLONESHOT 即同一套接字由一个进程管理 默认开启
 *  iTRIGMode: 触发模式 默认TRIG_MODE_LT
 *  bNonBlock: 是否开启非阻塞 默认不开启
 */
void LUtil::addfd(int iEpollfd, int iFd, bool bOneShot, int iTRIGMode, bool bNonBlock) {
    epoll_event event;
    event.data.fd = iFd;

    if (TRIG_MODE_ET == iTRIGMode) // ET
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (bOneShot)
        event.events |= EPOLLONESHOT;

    epoll_ctl(iEpollfd, EPOLL_CTL_ADD, iFd, &event);

    // 设置非阻塞 listen最好设置成非阻塞
    // 若阻塞，LT模式下则一个客户端连接可能会唤醒多次epoll_wait,
    // 导致占用线程，未成功连接的线程会阻塞在accept
    if (bNonBlock) {
        setnonblocking(iFd);
    }
}