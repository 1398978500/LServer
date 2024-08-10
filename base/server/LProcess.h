#ifndef __LPROCESS_H__
#define __LPROCESS_H__

#include <string>

class LProcess {
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;

public:
    LProcess() = default;
    LProcess(LProcess&) = default;
    LProcess(std::string szIp, int iPort, int iSockFd, int iConnMode);
    ~LProcess();

    int read();
    void process();

public:
    std::string m_szIp;
    int m_iPort;

private:
    int m_iSockFd;
    int m_iConnMode;

    int m_iReadIdx;
    char m_readBuf[READ_BUFFER_SIZE];
    int m_iWriteIdx;
    char m_writeBuf[WRITE_BUFFER_SIZE];
};

#endif
