#include "TcpClient.h"
#include "unistd.h"
#include "string.h"
#include "strings.h"

void readCallback(EventLoop *eventLoop, int fd, void *args) {
    TcpClient *client = (TcpClient *)args;
    client->doReceiveData();
}


void writeCallback(EventLoop *evetLoop, int fd, void *args) {
    TcpClient *client = (TcpClient *)args;
    client->doSendData();
}

void connectionDelay(EventLoop *eventLoop,int fd,void *args) {
    TcpClient *client = (TcpClient *)args;
    eventLoop->delEvent(fd);

    int result = 0;
    socklen_t result_len = sizeof(result);
    getsockopt(fd,SOL_SOCKET,SO_ERROR,&result,&result_len);

    if (result == 0) {
        client->m_connectStatus = true;
        if (client->m_onCliConnect) {
            client->m_onCliConnect(client, nullptr);
        }
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

    this->doConnect();
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
        m_connectStatus = true;
        if (m_onCliConnect) {
            m_onCliConnect(this, nullptr);
        }
        m_eventLoop->addEvent(m_sockFD,readCallback,EPOLLIN,this);
    }
    
    return false;
}

bool TcpClient::doDisConnect() {
    if (m_sockFD != -1) {
        m_eventLoop->delEvent(m_sockFD);
        close(m_sockFD);
    }
    
    m_connectStatus = false;
    if (m_onCliClose) {
        m_onCliClose(this,nullptr);
    }
    

    this->doConnect();
}

int TcpClient::sendData(const char *data, unsigned int dataLen) {
    if (!m_connectStatus) {
        fprintf(stderr,"connection lost");
        return -1;
    }

    m_sendBuf.sendData(data,dataLen);
    m_eventLoop->addEvent(m_sockFD,writeCallback,EPOLLOUT,this);
}

int TcpClient::doSendData() {
    int nSend = 0;
    while (m_sendBuf.length()) {
        nSend = m_sendBuf.write2Fd(m_sockFD);
        if (nSend == -1) {
            fprintf(stderr,"do send data error");
            this->doDisConnect();
            return -1;
        } else if (nSend == 0) {
            break;
        }
    }

    if (!m_sendBuf.length()) {
        m_eventLoop->delEvent(m_sockFD,EPOLLOUT);
    }
    
    return nSend;
}

int TcpClient::doReceiveData() {
    int nRead = m_receiveBuf.receiveData(m_sockFD);
    if (nRead == 0) {
        fprintf(stderr,"close by peer");
        this->doDisConnect();
    }
    

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

    this->sendData(str.c_str(),sizeof(str));
    
    return nRead;
}