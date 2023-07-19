#include "src/EventLoop.h"
#include "src/TcpClient.h"
#include "src/TcpServer.h"

void onClientConnect(TcpClient * client,void *args) {
    client->sendData("test",4);
}

void onClientClose(TcpClient *client, void *args) {
    std::cout << "disconnect" << std::endl;
}

int main() {
    EventLoop loop;
    // TcpClient client(&loop,"100.64.0.8",9527,"test");
    // client.registerOnConnect(onClientConnect);
    // client.registerOnClose(onClientClose);
    TcpServer server(&loop,"127.0.0.1",9527);
    loop.processEvents();
    return 0;
}