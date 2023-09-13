#pragma once

#include "EventLoop.h"
#include "ChannelBuf.h"

class Channel {
private:
    int m_connfd;
    EventLoop *m_eventLoop;
    ReceiveBuf m_rBuf;
    SendBuf m_sBuf;
public:
    Channel(int connFD, EventLoop *eventLoop);
    ~Channel();

    void doRead();
    void doWrite();
    void close();
    int sendMsg(const char *data, int dataSize);
    int getFd();
};

typedef void (*ChannelCallback)(Channel *channel,void *args);