#include "src/EventLoop.h"
#include "src/TcpClient.h"

void onClientConnect(TcpClient * client,void *args) {
    client->sendData("test",4);
}

void onClientClose(TcpClient *client, void *args) {
    std::cout << "disconnect" << std::endl;
}

int main() {
    EventLoop loop;
    TcpClient client(&loop,"100.64.0.8",9527,"test");
    client.registerOnConnect(onClientConnect);
    client.registerOnClose(onClientClose);
    loop.processEvents();
}