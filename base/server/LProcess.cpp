#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <string>

#include "LProcess.h"
#include "LUtil.h"
#include "NanoLog.h"

LProcess::LProcess(std::string szIp, int iPort, int iSockFd, int iConnMode)
    : m_szIp(szIp), m_iPort(iPort), m_iSockFd(iSockFd), m_iConnMode(iConnMode), m_iReadIdx(0), m_iWriteIdx(0) {
    memset(m_readBuf, 0, READ_BUFFER_SIZE);
    memset(m_writeBuf, 0, WRITE_BUFFER_SIZE);
}

LProcess::~LProcess() {}

int LProcess::read() {
    if (m_iReadIdx >= READ_BUFFER_SIZE) {
        return false;
    }

    int iByteRead = 0;
    do {
        iByteRead = recv(m_iSockFd, m_readBuf, READ_BUFFER_SIZE - m_iReadIdx, 0);
        if (iByteRead <= 0) {
            return iByteRead;
        }
        m_iReadIdx += iByteRead;

    } while (m_iConnMode == TRIG_MODE_ET);

    LOG_INFO << "客户端消息 ip" << m_szIp << " port " << m_iPort << " 消息" << std::string(m_readBuf);

    process();

    return m_iReadIdx;
}

// 暂时收到什么就发回hello
void LProcess::process() {
    char str[] = "hello";
    strncpy(m_writeBuf, str, WRITE_BUFFER_SIZE - m_iWriteIdx);
    m_iWriteIdx = sizeof(str);
    int iByte = send(m_iSockFd, m_writeBuf, m_iWriteIdx, 0);
    if (iByte <= 0) {
        return;
    }

    m_iWriteIdx -= iByte;
    m_writeBuf[m_iWriteIdx] = '\0';

    // 清空读缓冲区
    m_iReadIdx = 0;
    memset(m_readBuf, 0, READ_BUFFER_SIZE);
}