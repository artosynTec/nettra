#include <sys/ioctl.h>
#include <sys/socket.h>
#include <cassert>
#include <cstring>

#include "ChannelBuf.h"
#include "ByteBufAllocator.h"
#include "Logger.h"

ChannelBuf::ChannelBuf() {
    m_buf = nullptr;
}

ChannelBuf::~ChannelBuf() {
    release();
}

const int ChannelBuf::length() const {
    return m_buf ? m_buf->length : 0;
}

void ChannelBuf::pop(int len) {
    m_buf->pop(len);

    if (m_buf->length == 0) {
        ByteBufAllocator::instance()->release(m_buf);
        m_buf = nullptr;
    }
}

void ChannelBuf::release() {
    if (m_buf)  {
        ByteBufAllocator::instance()->release(m_buf);
        m_buf = nullptr;
    }
}

char* ChannelBuf::data() const{
    return m_buf ? m_buf->data + m_buf->head : nullptr;
}

void ReceiveBuf::adjust() {
    if (m_buf) {
        m_buf->adjust();
    }
}

int ReceiveBuf::receiveData(int fd) {
    int allCanRead = 0;

    if (ioctl(fd,FIONREAD,&allCanRead) == -1){
        LOG_ERROR("ioctl error,allCanRead is :%d",allCanRead);
        return -1;
    }

    LOG_TRACE("all can read is %d",allCanRead);
    
    if (!m_buf) {
        m_buf = ByteBufAllocator::instance()->allocBuf(allCanRead);
        if (!m_buf) {
            LOG_ERROR("buf alloc failed");
            return -1;
        }
    } else {
        assert(m_buf->head == 0);
        if (m_buf->writeableBytes() < allCanRead) {
            ByteBuf *newBuf = ByteBufAllocator::instance()->allocBuf(allCanRead + m_buf->length);
            if (!newBuf) {
                fprintf(stderr,"buf alloc failed");
                return -1;
            }

            newBuf->copy(m_buf);
            ByteBufAllocator::instance()->release(m_buf);
            m_buf = newBuf;
        }
    }
    
    int nRead = 0;

    do {
        if (allCanRead == 0) {
            nRead = recv(fd,m_buf->data + m_buf->length, M1K,MSG_WAITALL);
        } else {
            nRead = recv(fd,m_buf->data + m_buf->length,allCanRead,MSG_WAITALL);
        }
    } while (nRead  == -1 && errno == EINTR);
    
    if (nRead > 0) {
        if (allCanRead != 0) {
            assert(nRead == allCanRead);
        }
        
        m_buf->length += nRead;
    }
    
    return nRead;
}

int SendBuf::sendData(const char *data, int dataSize) {
    if (!m_buf) {
        m_buf = ByteBufAllocator::instance()->allocBuf(dataSize);
        if(!m_buf) {
            fprintf(stderr,"alloc buf error");
            return -1;
        }
    } else {
        assert(m_buf->head == 0);
        
        if (m_buf->writeableBytes() < dataSize) {
            ByteBuf *newBuf = ByteBufAllocator::instance()->allocBuf(m_buf->length + dataSize);
            if (!newBuf) {
                fprintf(stderr,"buf alloc error");
                return -1;
            }

            newBuf->copy(m_buf);
            ByteBufAllocator::instance()->release(m_buf);
            m_buf = newBuf;
        }
        
    }

    memcpy(m_buf->data + m_buf->length,data,dataSize);
    m_buf->length += dataSize;

    return 0;
}

int SendBuf::write2Fd(int fd) {
    assert(m_buf != nullptr && m_buf->head == 0);

    int nWrite = 0;

    do {
        nWrite = send(fd,m_buf->data,m_buf->length,MSG_NOSIGNAL);
    } while (nWrite == -1 && errno == EINTR);


    if (nWrite > 0) {
        m_buf->pop(nWrite);
        m_buf->adjust();
    }
    
    if (nWrite == -1 && errno == EAGAIN) {
        nWrite = 0;
    }
    
    return nWrite;
}