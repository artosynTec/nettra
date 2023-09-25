#include "fcntl.h"
#include "sys/socket.h"
#include "netinet/tcp.h"
#include "netinet/in.h"
#include <unistd.h>

#include "Channel.h"
#include "Logger.h"

void channelReadCallback(EventLoop *eventLoop,int fd,void *args) {
    Channel *channel = (Channel *)args;
    channel->doRead();
}


Channel::Channel(int connFD, EventLoop *eventLoop) {
    m_connfd = connFD;
    m_eventLoop = eventLoop;

    int flag = fcntl(m_connfd,F_GETFL,0);//file status flag
    fcntl(connFD, F_SETFL, O_NONBLOCK | flag);

    int op = 1;
    setsockopt(m_connfd,IPPROTO_TCP,TCP_NODELAY,&op,sizeof(op));

    m_eventLoop->addEvent(m_connfd, channelReadCallback, EPOLLIN, this);
}

Channel::~Channel() {

}

int Channel::getFd() {
    return m_connfd;
}

// todo 未完成，需要设置解码规则
void Channel::doRead() {
    int nRead = m_rBuf.receiveData(m_connfd);

    if (nRead == -1) {
        LOG_ERROR("read data from socket error");
        this->close();
        return;
    } else if (nRead == 0) {
        LOG_INFO("connection closed by peer");
        this->close();
        return;
    } else {
        const char *data = m_rBuf.data();
        int length = m_rBuf.length();

        // std::string str("");
        // std::string str2("0123456789abcdef");
        // for (int i = 0; i < length; i++) {
        //     int b;
        //     b = 0x0f & (data[i] >> 4);
        //     char s1 = str2.at(b);
        //     str.append(1, str2.at(b));
        //     b = 0x0f & data[i];
        //     str.append(1, str2.at(b));
        //     char s2 = str2.at(b);
        // }

        // std::cout << str << std::endl;

        m_rBuf.release();
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