#include <sys/socket.h>
#include <sys/types.h>

#include "LProcess.h"
#include "LUtil.h"

LProcess::LProcess() {}

LProcess::~LProcess() {}

bool LProcess::readOnce() {
    int iByteRead = 0;
    do {
        iByteRead = recv(m_iSockFd, m_readBuf, READ_BUFFER_SIZE - m_iReadIdx, 0);
        
    } while (m_iTrigMode == TRIG_MODE_ET);
}