#ifndef NETHZ_STRUCTCOLLECTION_H
#define NETHZ_STRUCTCOLLECTION_H

#include "functional"

class EventLoop;
using ioCallback = std::function<void(EventLoop *eventLoop, int fd, void *args)>;


struct IoEvent {
    int mask;
    void *readArgs;
    void *writeArgs;
    ioCallback *readCallback;
    ioCallback *writeCallback;
};

#endif