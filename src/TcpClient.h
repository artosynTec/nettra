#ifndef NETHZ_TCPCLIENT_H
#define NETHZ_TCPCLIENT_H

#include "netinet/in.h"
#include "strings.h"
#include "sys/socket.h"
#include <arpa/inet.h>
#include "iostream"
#include "functional"

#include "EventLoop.h"
#include "ChannelBuf.h"

class TcpClient {
public:
    using onCliConnect = std::function<void(TcpClient *tcpClient, void *args)>;
    using onCliClose = std::function<void(TcpClient *tcpClient, void *args)>;

    TcpClient(EventLoop *eventLoop, char *ip, unsigned int port, const char *clientName);
    bool doConnect();
    int sendData(const char *data, unsigned int dataLen);
    int doSendData();
    int doReceiveData();
    bool doDisConnect();
    bool getStatus();

    void registerOnConnect(onCliConnect callBack) {
        m_onCliConnect = callBack;
    }

    void registerOnClose(onCliClose callBack) {
        m_onCliClose = callBack;
    }

    onCliConnect m_onCliConnect;
    onCliClose m_onCliClose;
public:
    bool m_connectStatus;
    ReceiveBuf m_receiveBuf;
    SendBuf m_sendBuf;
private:
    void encode();
    void decode();
private:
    int m_sockFD;
    sockaddr_in m_serverAddr;
    const char *m_clientName;
    EventLoop *m_eventLoop;
};

#endif // NETHZ_TCPCLIENT_H
