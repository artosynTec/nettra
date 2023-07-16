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
};


class ReceiveBuf : public ChannelBuf{
public:
    int receiveData(int fd);
    const char *data() const;
    void adjust();
};

class SendBuf : public ChannelBuf {
public:
    int sendData(const char *data, int dataSize);
    int write2Fd(int fd);
};
