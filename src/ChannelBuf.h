#pragma once

#include "ByteBuf.h"
#include "ByteBufAllocator.h"


class ChannelBuf {
protected:
    ByteBuf *m_buf;
public:
    ChannelBuf();
    ~ChannelBuf();

    const int Length() const;

    void Pop(int len);

    void Clear();
};


class ReceivBuf : public ChannelBuf{
public:
    int ReadData(int fd);
    const char *Data() const;
    void Adjust();
};

class SendBuf : public ChannelBuf {
public:
    int SendData(const char *data,int dataSize);
    int Write2fd(int fd);
};
