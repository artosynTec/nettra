#include <pthread.h>
#include "ThreadQueue.h"
#include "TaskMsg.h"

class ThreadPool {
private:
    ThreadQueue<TaskMsg> **queues;
    int m_threadCount;
    pthread_t *threadId;
    int m_index;
public:
    ThreadPool(int threadCount);
    ~ThreadPool();
public:
    ThreadQueue<TaskMsg> *getThread();
    void sendTask(taskFun func,void *args = nullptr);
};