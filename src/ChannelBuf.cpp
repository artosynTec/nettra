#include <sys/ioctl.h>
#include <sys/socket.h>
#include <assert.h>
#include "string.h"

#include "ChannelBuf.h"
#include "ByteBufAllocator.h"

ChannelBuf::ChannelBuf() {
    m_buf = nullptr;
}

ChannelBuf::~ChannelBuf() {
    Clear();
}

const int ChannelBuf::Length() const {
    return m_buf ? m_buf->length : 0;
}

void ChannelBuf::Pop(int len) {
    m_buf->Pop(len);

    if (m_buf->length == 0) {
        ByteBufAllocator::instance()->Release(m_buf);
        m_buf = nullptr;
    }
}

void ChannelBuf::Clear() {
    if (m_buf)  {
        ByteBufAllocator::instance()->Release(m_buf);
        m_buf = nullptr;
    }
}

void ReceivBuf::Adjust() {
    if (m_buf) {
        m_buf->Adjust();
    }
}

const char* ReceivBuf::Data() const{
    return m_buf ? m_buf->data + m_buf->head : nullptr;
}

int ReceivBuf::ReadData(int fd) {
    int allCanRead;

    if (ioctl(fd,FIONREAD,&allCanRead) == -1){
        fprintf(stderr,"ioctl error");
        return -1;
    }
    
    if (!m_buf) {
        m_buf = ByteBufAllocator::instance()->AllocBuf(allCanRead);
        if (!m_buf) {
            fprintf(stderr,"buf alloc failed");
            return -1;
        }
    } else {
        assert(m_buf->head == 0);
        if (m_buf->WriteableBytes() < allCanRead) {
            ByteBuf *newBuf = ByteBufAllocator::instance()->AllocBuf(allCanRead + m_buf->length);
            if (!newBuf) {
                fprintf(stderr,"buf alloc failed");
                return -1;
            }
            
            newBuf->Copy(m_buf);
            ByteBufAllocator::instance()->Release(m_buf);
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

int SendBuf::SendData(const char *data,int dataSize) {
    if (!m_buf) {
        m_buf = ByteBufAllocator::instance()->AllocBuf(dataSize);
        if(!m_buf) {
            fprintf(stderr,"alloc buf error");
            return -1;
        }
    } else {
        assert(m_buf->head == 0);
        
        if (m_buf->WriteableBytes() < dataSize) {
            ByteBuf *newBuf = ByteBufAllocator::instance()->AllocBuf(m_buf->length + dataSize);
            if (!newBuf) {
                fprintf(stderr,"buf alloc error");
                return -1;
            }
            
            newBuf->Copy(m_buf);
            ByteBufAllocator::instance()->Release(m_buf);
            m_buf = newBuf;
        }
        
    }

    memcpy(m_buf->data + m_buf->length,data,dataSize);
    m_buf->length += dataSize;

    return 0;
}

int SendBuf::Write2fd(int fd) {
    assert(m_buf != nullptr && m_buf->head == 0);

    int nWrite = 0;

    do {
        nWrite = send(fd,m_buf->data,m_buf->length,MSG_NOSIGNAL);
    } while (nWrite == -1 && errno == EINTR);


    if (nWrite > 0) {
        m_buf->Pop(nWrite);
        m_buf->Adjust();
    }
    
    if (nWrite == -1 && errno == EAGAIN) {
        nWrite = 0;
    }
    
    return nWrite;
}