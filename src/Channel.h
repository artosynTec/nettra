#pragma once

#include "EventLoop.h"
#include "ChannelBuf.h"

class Channel {
private:
    int m_connfd;
    EventLoop *m_eventLoop;
    ReceivBuf m_rBuf;
    SendBuf m_sBuf;
public:
    Channel(int connfd, EventLoop *eventLoop);
    ~Channel();

    void DoRead();
    void DoWrite();
    void Close();
    int SendMsg(const char *data,int dataSize);
    int Getfd();
};
