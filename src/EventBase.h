#ifndef NETHZ_EVENTBASE_H
#define NETHZ_EVENTBASE_H

class EventLoop;


typedef void ioCallback(EventLoop *eventLoop, int fd, void *args);

struct ioEvent {
    ioEvent() : readCallback(nullptr),writeCallback(nullptr),rcbArgs(nullptr),wcbArgs(nullptr){};

    int mask;
    ioCallback *readCallback;
    ioCallback *writeCallback;

    void *rcbArgs;
    void* wcbArgs;
};

#endif //NETHZ_EVENTBASE_H
