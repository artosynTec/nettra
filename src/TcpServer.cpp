#include "TcpServer.h"
#include "Channel.h"
#include "Logger.h"


#include "sys/socket.h"
#include "arpa/inet.h"
#include "strings.h"
#include "unistd.h"
#include "thread"


int TcpServer::m_maxConns = 1024;
int TcpServer::m_currentConns = 0;
pthread_mutex_t TcpServer::m_connsMutex = PTHREAD_MUTEX_INITIALIZER;
Channel ** TcpServer::m_channels = nullptr;

void acceptCallback(EventLoop *eventLoop, int fd, void *args) {
    TcpServer *server = (TcpServer *) args;
    server->doAccept();
}

TcpServer::~TcpServer() {
    m_eventLoop->delEvent(m_sockFd);
    close(m_sockFd);
    m_sockFd = -1;
}

TcpServer::TcpServer(EventLoop *eventLoop, const char *ip, uint16_t port) {
    bzero(&m_connAddr,sizeof(m_connAddr));

    m_sockFd = socket(AF_INET,SOCK_STREAM | SOCK_CLOEXEC,IPPROTO_TCP);
    if (m_sockFd == -1) {
        LOG_ERROR("create socket error");
        exit(1);
    }
    
    sockaddr_in serverAddr{};
    bzero(&serverAddr,sizeof(serverAddr));
    
    serverAddr.sin_family = AF_INET;
    inet_aton(ip,&serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    int rBuf = M16K;
    if (setsockopt(m_sockFd,SOL_SOCKET,SO_RCVBUF,&rBuf,sizeof(rBuf)) < 0) {
        LOG_ERROR("set receive buffer failed");
        exit(1);
    }

    int op = 1;
    if (setsockopt(m_sockFd,SOL_SOCKET,SO_REUSEADDR,&op,sizeof(op)) < 0) {
        LOG_ERROR("set reuse addr failed");
        exit(1);
    }

    int bindRet = bind(m_sockFd,(const sockaddr *) &serverAddr,sizeof(serverAddr));
    if (bindRet < 0) {
        LOG_ERROR("socket bind failed");
        exit(1);
    }
    
    int listenRet = listen(m_sockFd,500);
    if (listenRet < 0) {
        LOG_ERROR("socket listen failed");
        exit(1);
    }

    // stdin stdout stderr
    m_channels = new Channel *[m_maxConns + 3];
    if (!m_channels) {
        LOG_ERROR("create channels array failed");
        exit(1);
    }


    bzero(m_channels,sizeof(Channel *)*(m_maxConns + 3));


    int systemThreadNum = (int)std::thread::hardware_concurrency();
    m_threadPool = new ThreadPool(systemThreadNum * 2);
    if (!m_threadPool) {
        LOG_ERROR("create thread pool failed");
        exit(1);
    }

    LOG_INFO("thread pool has init,thread count is :%d",systemThreadNum * 2);
    
    
    m_eventLoop = eventLoop;

    m_eventLoop->addEvent(m_sockFd,acceptCallback,EPOLLIN,this);
}

void TcpServer::doAccept() {
    int connFD;

    while (true) {
        connFD = accept(m_sockFd, (sockaddr *) &m_connAddr, &m_addrLen);
        if (connFD == -1) {
            if (errno == EINTR) {
                LOG_WARN("accept failed,errno:%d",errno);
                continue;
            } else if (errno == EMFILE) {
                LOG_WARN("accept failed,errno:%d",errno);
                break;
            } else if (errno == EAGAIN) {
                LOG_WARN("accept failed,errno:%d",errno);
                break;
            } else {
                LOG_ERROR("accept failed,errno:%d",errno);
                exit(1);
            }
        } else {
            LOG_INFO("client connect,connect fd is :%d",connFD);
            int currentConns;
            getConnNum(&currentConns);

            // todo
            if (currentConns >= std::thread::hardware_concurrency() * 2) {
                LOG_ERROR("too many conns:%d",currentConns);
                close(connFD);
            } else {
                if (m_threadPool) {
                    ThreadQueue<TaskMsg> *queue = m_threadPool->getThread();
                    TaskMsg taskMsg{};
                    taskMsg.type = TaskMsg::NEW_CONN;
                    taskMsg.connfd = connFD;
                    queue->send(taskMsg);
                } else {
                    new Channel(connFD, m_eventLoop);
                    break;
                }
            }
        }
    }
}

void TcpServer::getConnNum(int *currConn) {
    pthread_mutex_lock(&m_connsMutex);
    *currConn = m_currentConns;
    pthread_mutex_unlock(&m_connsMutex);
}

void TcpServer::decreaseConn(int connFd) {
    pthread_mutex_lock(&m_connsMutex);
    m_channels[connFd] = nullptr;
    m_currentConns--;
    pthread_mutex_unlock(&m_connsMutex);
}

void TcpServer::increaseConn(int connFd,Channel *conn) {
    pthread_mutex_lock(&m_connsMutex);
    m_channels[connFd] = conn;
    m_currentConns++;
    pthread_mutex_unlock(&m_connsMutex);
}