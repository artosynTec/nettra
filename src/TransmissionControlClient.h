#ifndef NETHZ_TRANSMISSIONCONTROLCLIENT_H
#define NETHZ_TRANSMISSIONCONTROLCLIENT_H

#include "netinet/in.h"
#include "strings.h"
#include "sys/socket.h"
#include <arpa/inet.h>
#include "iostream"
#include "functional"

class TransmissionControlClient {
public:
    using onCliConnect = std::function<void(TransmissionControlClient *tcpClient, void *args)>;
    using onCliClose = std::function<void(TransmissionControlClient *tcpClient, void *args)>;

    TransmissionControlClient(const char *ip, unsigned int port);
    bool doConnect();
    int doSendData(const char *data, unsigned int dataLen);
    int doReceiveData();
    bool doDisConnect();
    bool getStatus();

    void registerOnConnect(onCliConnect callBack) {
        m_onCliConnect = callBack;
    }

    void registerOnClose(onCliClose callBack) {
        m_onCliClose = callBack;
    }

private:
    void encode();
    void decode();
private:
    int m_sockFD;
    bool m_connectStatus;
    sockaddr_in m_serverAddr;
    onCliConnect m_onCliConnect;
    onCliClose m_onCliClose;
};

#endif // NETHZ_TRANSMISSIONCONTROLCLIENT_H
