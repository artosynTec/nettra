#include "netinet/in.h"

#include "EventLoop.h"
#include "ThreadPool.h"
#include "Channel.h"

class TcpServer {
private:
    int m_sockFd;
    EventLoop *m_eventLoop;
    sockaddr_in m_connAddr;
    socklen_t m_addrLen;
    ThreadPool *m_threadPool;

    static int m_maxConns;
    static int m_currentConns;
    static pthread_mutex_t m_connsMutex;

public:
    TcpServer(EventLoop *eventLoop,const char *ip, uint16_t port);
    ~TcpServer();

    void doAccept();

    void addMsgRouter();

    ThreadPool *threadPool();

    static void increaseConn(int connFd, Channel *channel);
    static void decreaseConn(int connFd);
    static void getConnNum(int *currentConn);
    static Channel **m_channels;

    static void setOnConnect(ChannelCallback ccb, void *args = nullptr);
    static void setOnDisConnect(ChannelCallback ccb, void *args = nullptr);

    static ChannelCallback onConnectCallback;
    static void *onConnectCallbackArgs;

    static ChannelCallback onDisConnectCallback;
    static void *onDisConnectCallbackArgs;
};