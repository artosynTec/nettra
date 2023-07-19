#include "netinet/in.h"

#include "EventLoop.h"

class TcpServer {
private:
    int m_sockFd;
    EventLoop *m_eventLoop;
    sockaddr_in m_connAddr;
    socklen_t m_addrLen;
public:
    TcpServer(EventLoop *eventLoop,const char *ip, uint16_t port);
    ~TcpServer();

    void doAccept();
};
