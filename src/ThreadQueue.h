#include "EventLoop.h"
#include "Logger.h"

#include <queue>
#include <pthread.h>
#include <sys/eventfd.h>
#include <cstdio>
#include <unistd.h>


template<typename T>

class ThreadQueue {
private:
    int evFD;
    EventLoop *m_loop;
    std::queue<T> evQueue;
    pthread_mutex_t evQueueMutex;
public:
    ThreadQueue() {
        m_loop = nullptr;
        pthread_mutex_init(&evQueueMutex,nullptr);
        evFD = eventfd(0, EFD_NONBLOCK);
        if (evFD == -1) {
            LOG_ERROR("create event fd error");
            exit(1);
        }
    }


    ~ThreadQueue() {
        LOG_TRACE("ThreadQueue destructor called");
        pthread_mutex_destroy(&evQueueMutex);
        close(evFD);
        delete m_loop;
    }

    void send(const T &task) {
        unsigned long long idleNum = 1;
        pthread_mutex_lock(&evQueueMutex);
        evQueue.push(task);

        int ret = write(evFD, &idleNum, sizeof(unsigned long long));
        if (ret == -1) {
            LOG_ERROR("evFD write error");
            exit(1);
        }
        
        pthread_mutex_unlock(&evQueueMutex);
    }

    void recv(std::queue<T> &newQueue) {
        unsigned long long idleNum = 1;
        pthread_mutex_lock(&evQueueMutex);
        int ret = read(evFD, &idleNum, sizeof(unsigned long long));
        if (ret == -1) {
            LOG_ERROR("evFD read error");
            exit(1);
        }
        
        std::swap(newQueue,evQueue);
        pthread_mutex_unlock(&evQueueMutex);
    }

    void setLoop(EventLoop *pLoop) {
        this->m_loop = pLoop;
    }

    EventLoop *getLoop() {
        return this->m_loop;
    }

    void setCallback(ioCallback *cb,void *args = nullptr) {
        if (m_loop) {
            m_loop->addEvent(evFD, cb, EPOLLIN, args);
        }
    }
};