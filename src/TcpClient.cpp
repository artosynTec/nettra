#include "TcpClient.h"

void readCallback(EventLoop *eventLoop, int fd, void *args) {
    TcpClient *client = (TcpClient *)args;
    client->doReceiveData();
}


void connectionDelay(EventLoop *eventLoop,int fd,void *args) {
    TcpClient *client = (TcpClient *)args;
    eventLoop->delEvent(fd);

    int result = 0;
    socklen_t result_len = sizeof(result);
    getsockopt(fd,SOL_SOCKET,SO_ERROR,&result,&result_len);

    if (result == 0) {
        client->m_connectStatus = true;
        eventLoop->addEvent(fd,readCallback,EPOLLIN,client);
    } else  {
        fprintf(stderr,"connect error");
    }
}


TcpClient::TcpClient(EventLoop *eventLoop, char *ip, unsigned int port, const char *clientName) {
    m_eventLoop = eventLoop;
    m_clientName = clientName;
    m_connectStatus = false;
    bzero(&m_serverAddr,sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(port);
    int ret = inet_aton(ip,&m_serverAddr.sin_addr);
    if (ret == 0) {
        std::cout << "ip:" << ip << ";port:" << port << std::endl;
    }
}

bool TcpClient::doConnect() {
    m_sockFD = socket(AF_INET,SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,IPPROTO_TCP);
    if (m_sockFD == -1) {
        std::cout << "create socket error" << std::endl;
        return false;
    }

    int connectRet = connect(m_sockFD,(sockaddr *) &m_serverAddr,sizeof(m_serverAddr));
    if (connectRet < 0) {
        if (errno == EINPROGRESS) {
            m_eventLoop->addEvent(m_sockFD,connectionDelay,EPOLLOUT,this);
        } else {
            std::cout << "connect errno:" <<  errno << std::endl;
        }
    } else {
        m_eventLoop->addEvent(m_sockFD,readCallback,EPOLLIN,this);
        m_connectStatus;
    }
    
    return false;
}

int TcpClient::doReceiveData() {
    m_receiveBuf.receiveData(m_sockFD);

    const char *data = m_receiveBuf.data();
    int length = m_receiveBuf.length();

    std::string str("");
    std::string str2("0123456789abcdef");
    for (int i = 0; i < length; i++) {
        int b;
        b = 0x0f & (data[i] >> 4);
        char s1 = str2.at(b);
        str.append(1, str2.at(b));
        b = 0x0f & data[i];
        str.append(1, str2.at(b));
        char s2 = str2.at(b);
    }

    std::cout << str << std::endl;

    m_receiveBuf.release();
}