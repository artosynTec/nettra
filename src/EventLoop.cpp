#include "EventLoop.h"
#include "sys/epoll.h"
#include "unistd.h"
#include "iostream"
#include <cassert>

EventLoop::EventLoop() {
    m_epollFD = epoll_create(1);
    if (m_epollFD == -1) {
        fprintf(stderr,"epoll_create failed");
    }
}

EventLoop::~EventLoop() {
    close(m_epollFD);
}

void EventLoop::addEvent(int fd, ioCallback *callback, int mask, void *args) {
    int finalMask;
    int op;

    eventMapIterator it = m_eventMap.find(fd);
    if (it == m_eventMap.end()) {
        finalMask = mask;
        op = EPOLL_CTL_ADD;
    } else {
        finalMask = it->second.mask | mask;
        op = EPOLL_CTL_MOD;
    }
    

    if (mask & EPOLLIN) {
        m_eventMap[fd].readCallback = callback;
        m_eventMap[fd].rcbArgs = args;
    } else if (mask & EPOLLOUT) {
        m_eventMap[fd].writeCallback = callback;
        m_eventMap[fd].wcbArgs = args;
    }
    
    m_eventMap[fd].mask = finalMask;
    epoll_event event{};
    event.events = finalMask;
    event.data.fd = fd;

    if (epoll_ctl(m_epollFD,op,fd,&event) == -1) {
        fprintf(stderr,"epoll_ctl failed");
    }
    
    m_listenedFdSet.insert(fd);
}

void EventLoop::processEvents() {
    while (true) {
        eventMapIterator it;

        int nfds = epoll_wait(m_epollFD, m_events, 10, -1);

        for (int i = 0; i < nfds; ++i) {
            it = m_eventMap.find(m_events[i].data.fd);
            assert(it != m_eventMap.end());

            ioEvent &event = it->second;

            if (m_events[i].events & EPOLLIN) {
                void *args = event.rcbArgs;
                event.readCallback(this,m_events[i].data.fd,args);
            } else if (m_events[i].events & EPOLLOUT) {
                void *args = event.wcbArgs;
                event.writeCallback(this,m_events[i].data.fd,args);
            } else if (m_events[i].events &(EPOLLHUP | EPOLLERR)) {
                if (event.readCallback) {
                    void *args = event.rcbArgs;
                    event.readCallback(this,m_events[i].data.fd,args);
                } else if (event.writeCallback) {
                    void *args = event.wcbArgs;
                    event.writeCallback(this,m_events[i].data.fd,args);
                } else {
                    this->delEvent(m_events[i].data.fd);
                }
            }
        }
    }
}

void EventLoop::delEvent(int fd) {
    m_eventMap.erase(fd);

    m_listenedFdSet.erase(fd);

    epoll_ctl(m_epollFD,EPOLL_CTL_DEL,fd, nullptr);
}


void EventLoop::delEvent(int fd, int mask) {
    eventMapIterator it = m_eventMap.find(fd);
    if (it == m_eventMap.end()) {
        return;
    }
    
    int originMask = it->second.mask;

    if (!originMask & (~mask)) {
        this->delEvent(fd);
    } else {
        epoll_event event{};
        event.events = originMask & (~mask);
        event.data.fd = fd;
        epoll_ctl(m_epollFD,EPOLL_CTL_MOD,fd,&event);
    }
}

void EventLoop::addTask(taskFun tf,void *args) {
    taskFuncPair funcPair(tf,args);
    m_readyTasks.push_back(funcPair);
}

void EventLoop::processTasks() {
    for (taskFuncPair tfp : m_readyTasks) {
        taskFun tf = tfp.first;
        void *args = tfp.second;

        tf(this,args);
    }
    m_readyTasks.clear();
}