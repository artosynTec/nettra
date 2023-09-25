#include "ThreadPool.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Logger.h"

#include <cstdio>
#include <thread>

void handleTask(EventLoop *loop,int fd,void *args) {
    auto *queue = (ThreadQueue<TaskMsg> *)args;
    std::queue<TaskMsg> tasks;
    queue->recv(tasks);
    printf("loop address in handleTask:%p\n",loop);

    while (!tasks.empty()) {
        TaskMsg task = tasks.front();

        tasks.pop();

        if (task.type == TaskMsg::NEW_CONN) {
            // memory leak!
            // The channel pointer must be guaranteed to be accessible even after this method ends, because the m_loop will use it later. So it must be a pointer rather than a local variable.
           auto *channel = new Channel(task.connfd,loop);
        } else if (task.type == TaskMsg::NEW_TASK) {
            loop->addTask(task.taskCallback,task.args);
        } else {
            LOG_ERROR("unknown task");
        }
    }
}


void *threadMain(void *args) {
    auto *queue = (ThreadQueue<TaskMsg> *)args;

    // memory leak!
    auto *loop = new EventLoop();
    printf("loop address in threadMain:%p\n",loop);
    
    queue->setLoop(loop);
    // todo 
    queue->setCallback(handleTask,queue);

    loop->processEvents();
    return nullptr;
}

ThreadPool::ThreadPool(int threadCount) {
    m_index = 0;
    queues = nullptr;
    m_threadCount = threadCount;

    if (threadCount <= 0) {
        LOG_ERROR("thread count must greater than 0,thread count now is :%d",threadCount);
        exit(1);
    }
    

    queues = new ThreadQueue<TaskMsg> *[threadCount];
    threadId = new pthread_t[threadCount];

    int ret;

    for (size_t i = 0; i < threadCount; i++) {
        queues[i] = new ThreadQueue<TaskMsg>();
        ret = pthread_create(&threadId[i],nullptr,threadMain,queues[i]);
        if (ret == -1) {
            LOG_ERROR("create thread error");
            exit(1);
        }
        
        pthread_detach(threadId[i]);
    }
}

ThreadQueue<TaskMsg> *ThreadPool::getThread() {
    if (m_index == m_threadCount) {
        m_index = 0;
    }
    
    return queues[m_index];
}

void ThreadPool::sendTask(taskFun tf,void *args) {
    TaskMsg taskMsg{};

    for (size_t i = 0; i < m_threadCount; i++) {
        taskMsg.type = TaskMsg::NEW_TASK;
        taskMsg.taskCallback = tf;
        taskMsg.args = args;

        ThreadQueue<TaskMsg> *queue = queues[i];
        queue->send(taskMsg);
    }
}

ThreadPool::~ThreadPool() {
    for (int i = 0; i < m_threadCount; ++i) {
        delete queues[i];
        queues[i] = nullptr;
    }

    delete[] queues;
    delete[] threadId;
}
