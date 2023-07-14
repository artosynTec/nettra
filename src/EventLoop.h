#ifndef NETHZ_EVENTLOOP_H
#define NETHZ_EVENTLOOP_H

#include <ext/hash_map>
#include <ext/hash_set>
#include <sys/epoll.h>

#include "EventBase.h"

using eventMap = __gnu_cxx::hash_map<int,ioEvent>;
using eventMapIterator = __gnu_cxx::hash_map<int,ioEvent>::iterator;
using listenFdSet = __gnu_cxx::hash_set<int>;


class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void AddEvent(int fd, ioCallback *callback, int mask, void *args);
    void DelEvent(int fd);
    void DelEvent(int fd,int mask);
    void ProcessEvents();
private:
    int m_epollFD;
    eventMap m_eventMap;
    listenFdSet m_listenedFdSet;
    epoll_event m_events[10];
};


#endif //NETHZ_EVENTLOOP_H
