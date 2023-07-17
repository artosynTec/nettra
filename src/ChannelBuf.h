#pragma once

#include "ByteBuf.h"
#include "ByteBufAllocator.h"


class ChannelBuf {
protected:
    ByteBuf *m_buf;
public:
    ChannelBuf();
    ~ChannelBuf();

    const int length() const;

    void pop(int len);

    void release();

    char *data() const;
};


class ReceiveBuf : public ChannelBuf{
public:
    int receiveData(int fd);
    
    void adjust();
};

class SendBuf : public ChannelBuf {
public:
    int sendData(const char *data, int dataSize);
    int write2Fd(int fd);
};
