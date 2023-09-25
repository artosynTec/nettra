#include "InetAddress.h"

#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

InetAddress::InetAddress(uint16_t port,std::string ip) {
    bzero(&addr_,sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::~InetAddress() {
    
}


std::string InetAddress::toIp() const {
    char buf[32] = {0};
    inet_ntop(AF_INET,&addr_.sin_addr.s_addr,buf,sizeof buf);
    return buf;
};


uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);
};


std::string InetAddress::toIpPort() const {
    char buf[32] = {0};
    inet_ntop(AF_INET,&addr_.sin_addr.s_addr,buf,sizeof buf);
    int end = strlen(buf);
    uint16_t port = toPort();
    sprintf(buf + end,":%u",port);

    return buf;
};