#include "src/EventLoop.h"
#include "src/TcpClient.h"

int main() {
    EventLoop loop;
    TcpClient client(&loop,"100.64.0.8",9527,"test");
    loop.processEvents();
}