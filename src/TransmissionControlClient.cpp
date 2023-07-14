#include "TransmissionControlClient.h"

TransmissionControlClient::TransmissionControlClient(const char *ip, unsigned int port) {
    m_connectStatus = false;
    bzero(&m_serverAddr,sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(port);
    int ret = inet_aton(ip,&m_serverAddr.sin_addr);
    if (ret == 0) {
        std::cout << "ip:" << ip << ";port:" << port << std::endl;
    }
}

bool TransmissionControlClient::DoConnect() {
    m_sockFD = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,IPPROTO_TCP);
    if (m_sockFD == -1) {
        std::cout << "create socket error" << std::endl;
        return false;
    }

    int connectRet = connect(m_sockFD,(sockaddr *) &m_serverAddr,sizeof(m_serverAddr));
    if (connectRet < 0) {
        if (errno == EINPROGRESS) {
            std::cout << "connect errno:" <<  errno << std::endl;
        } else {
            std::cout << "connect errno:" <<  errno << std::endl;
        }
    } else {
        m_connectStatus == true;
        std::cout << "connect success:" << std::endl;
    }
    

    
    return false;
}
