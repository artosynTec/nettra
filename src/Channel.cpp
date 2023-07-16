#include "fcntl.h"
#include "sys/socket.h"
#include "netinet/tcp.h"
#include "netinet/in.h"
#include <unistd.h>

#include "Channel.h"

void channelReadCallback(EventLoop *eventLoop,int fd,void *args) {
    Channel *channel = (Channel *)args;
    channel->doRead();
}


Channel::Channel(int connfd, EventLoop *eventLoop) {
    m_connfd = connfd;
    m_eventLoop = eventLoop;

    int flag = fcntl(m_connfd,F_GETFL,0);//file status flag
    fcntl(connfd,F_SETFL,O_NONBLOCK | flag);

    int op = 1;
    setsockopt(m_connfd,IPPROTO_TCP,TCP_NODELAY,&op,sizeof(op));

    m_eventLoop->addEvent(m_connfd, channelReadCallback, EPOLLIN, this);
}

int Channel::getFd() {
    return m_connfd;
}

// todo 未完成，需要设置解码规则
void Channel::doRead() {
    int nRead = m_rBuf.receiveData(m_connfd);

    if (nRead == -1) {
        fprintf(stderr,"read data from socket error");
        this->close();
        return;
    } else if (nRead == 0) {
        printf("connection closed by peer");
        this->close();
        return;
    }
    
    
}

void Channel::doWrite() {
    while (m_sBuf.length()) {
        int nSend = m_sBuf.write2Fd(m_connfd);
        if (nSend == -1) {
            fprintf(stderr,"write2fd error");
            this->close();
            return;
        } else if (nSend == 0) {
            break;
        }
    }
    
    if (m_sBuf.length() == 0) {
        m_eventLoop->delEvent(m_connfd, EPOLLOUT);
    }

    return;
}


void Channel::close() {
    m_eventLoop->delEvent(m_connfd);

    m_sBuf.release();
    m_rBuf.release();

    int fd = m_connfd;
    m_connfd = -1;
    ::close(fd);
}