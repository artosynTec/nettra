#include "TcpServer.h"
#include "Channel.h"
#include "string.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "strings.h"
#include "unistd.h"

void acceptCallback(EventLoop *eventLoop, int fd, void *args) {
    TcpServer *server = (TcpServer *) args;
    server->doAccept();
}

TcpServer::~TcpServer() {
    close(m_sockFd);
    m_sockFd = -1;
}

TcpServer::TcpServer(EventLoop *evetloop,const char *ip,uint16_t port) {
    bzero(&m_connAddr,sizeof(m_connAddr));

    m_sockFd = socket(AF_INET,SOCK_STREAM | SOCK_CLOEXEC,IPPROTO_TCP);
    if (m_sockFd == -1) {
        fprintf(stderr,"create socket error");
        exit(1);
    }
    
    sockaddr_in serverAddr;
    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_aton(ip,&serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    int bindRet = bind(m_sockFd,(const sockaddr *) &serverAddr,sizeof(serverAddr));
    if (bindRet < 0) {
        fprintf(stderr,"bind error");
        exit(1);
    }
    
    int listenRet = listen(m_sockFd,500);
    if (listenRet < 0) {
        fprintf(stderr,"listen error");
        exit(1);
    }
    
    m_eventLoop = evetloop;

    m_eventLoop->addEvent(m_sockFd,acceptCallback,EPOLLIN,this);
}

void TcpServer::doAccept() {
    int connfd;

    while (true) {
        connfd = accept(m_sockFd,(sockaddr *) &m_connAddr,&m_addrLen);
        if (connfd == -1) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EMFILE) {
                break;
            } else if (errno == EAGAIN) {
                break;
            } else {
                exit(1);
            }
        } else {
            std::cout << "conn success" << std::endl;
            new Channel(connfd,m_eventLoop);
            break;
        }
    }
}