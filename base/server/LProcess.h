#ifndef __LPROCESS_H__
#define __LPROCESS_H__

class LProcess {
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;

public:
    LProcess(/* args */);
    ~LProcess();

    bool readOnce();

private:
    int m_iTrigMode;
    int m_iActMode;
    int m_iSockFd;

    int m_iReadIdx;
    char m_readBuf[READ_BUFFER_SIZE];
    int m_iWriteIdx;
    char m_writeBuf[WRITE_BUFFER_SIZE];
};

#endif
