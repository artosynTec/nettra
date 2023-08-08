#include <queue>
#include <pthread.h>
#include <sys/eventfd.h>
#include <stdio.h>
#include <unistd.h>
#include "EventLoop.h"

template<typename T>

class ThreadQueue {
private:
    int evfd;
    EventLoop *loop;
    std::queue<T> evQueue;
    pthread_mutex_t evQueueMutex;
public:
    ThreadQueue() {
        loop = nullptr;
        pthread_mutex_init(&evQueueMutex,nullptr);
        evfd = eventfd(0,EFD_NONBLOCK);
        if (evfd == -1) {
            perror("create eventfd error");
            exit(1);
        }
    }
    ~ThreadQueue() {
        pthread_mutex_destroy(&evQueueMutex);
        close(evfd);
    }

    void send(const T &task) {
        unsigned long long idleNum = 1;
        pthread_mutex_lock(&evQueueMutex);
        evQueue.push(task);

        int ret = write(evfd,&idleNum,sizeof(unsigned long long));
        if (ret == -1) {
            perror("evfd write error");
        }
        
        pthread_mutex_unlock(&evQueueMutex);
    }

    void recv(std::queue<T> &newQueue) {
        unsigned long long idleNum = 1;
        pthread_mutex_lock(&evQueueMutex);
        int ret = read(evfd,&idleNum,sizeof(unsigned long long));
        if (ret == -1) {
            perror("evfd read error");
        }
        
        std::swap(newQueue,evQueue);
        pthread_mutex_unlock(&evQueueMutex);
    }

    void setLoop(EventLoop *loop) {
        this->loop = loop;
    }

    EventLoop *getLoop() {
        return this->loop;
    }

    void setCallback(ioCallback *cb,void *args = nullptr) {
        if (loop) {
            loop->addEvent(eventfd,cb,EPOLLIN,args);
        }
    }
};