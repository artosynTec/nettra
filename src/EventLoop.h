#ifndef NETHZ_EVENTLOOP_H
#define NETHZ_EVENTLOOP_H

#include <ext/hash_map>
#include <ext/hash_set>
#include <sys/epoll.h>

#include "EventBase.h"
#include "TaskMsg.h"

using eventMap = __gnu_cxx::hash_map<int,ioEvent>;
using eventMapIterator = __gnu_cxx::hash_map<int,ioEvent>::iterator;
using listenFdSet = __gnu_cxx::hash_set<int>;


class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void addEvent(int fd, ioCallback *callback, int mask, void *args);
    void addTask(taskFun tf,void *args);
    void delEvent(int fd);
    void delEvent(int fd, int mask);
    void processEvents();
    void processTasks();
private:
    int m_epollFD;
    eventMap m_eventMap;
    listenFdSet m_listenedFdSet;
    epoll_event m_events[10];

    typedef std::pair<taskFun,void *> taskFuncPair;
    std::vector<taskFuncPair> m_readyTasks;
};


#endif //NETHZ_EVENTLOOP_H
