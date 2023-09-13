#include "ThreadPool.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Logger.h"

#include <cstdio>

void handleTask(EventLoop *loop,int fd,void *args) {
    auto *queue = (ThreadQueue<TaskMsg> *)args;
    std::queue<TaskMsg> tasks;
    queue->recv(tasks);

    while (!tasks.empty()) {
        TaskMsg task = tasks.front();

        tasks.pop();

        if (task.type == TaskMsg::NEW_CONN) {
            // memory leak!
           auto *channel = new Channel(task.connfd,loop);
           if (!channel) {
            fprintf(stderr,"create channel failed");
            exit(1);
           }
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

    if (!loop) {
        fprintf(stderr,"create eventloop failed");
        exit(1);
    }
    
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
    delete[] queues;
    delete[] threadId;
}
