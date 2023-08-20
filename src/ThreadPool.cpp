#include "ThreadPool.h"
#include "EventLoop.h"
#include "Channel.h"

#include <unistd.h>
#include <stdio.h>

void handleTask(EventLoop *loop,int fd,void *args) {
    ThreadQueue<TaskMsg> *queue = (ThreadQueue<TaskMsg> *)args;
    std::queue<TaskMsg> tasks;
    queue->recv(tasks);

    while (!tasks.empty()) {
        TaskMsg task = tasks.front();

        tasks.pop();

        if (task.type == TaskMsg::NEW_CONN) {
           Channel *channel = new Channel(task.connfd,loop);
           if (!channel) {
            fprintf(stderr,"create channel failed");
            exit(1);
           }
        } else if (task.type == TaskMsg::NEW_TASK) {
            loop->addTask(task.taskCallback,task.args);
        } else {
            fprintf(stderr,"unkown task\n");
        }  
    }
}


void *threadMain(void *args) {
    ThreadQueue<TaskMsg> *queue = (ThreadQueue<TaskMsg> *)args;

    EventLoop *loop = new EventLoop();

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
        fprintf(stderr,"thread count must greater than 0");
        exit(1);
    }
    

    queues = new ThreadQueue<TaskMsg> *[threadCount];
    threadId = new pthread_t[threadCount];

    int ret;

    for (size_t i = 0; i < threadCount; i++) {
        queues[i] = new ThreadQueue<TaskMsg>();
        ret = pthread_create(&threadId[i],nullptr,threadMain,queues[i]);
        if (ret == -1)
        {
            perror("create thread error");
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
    TaskMsg taskMsg;

    for (size_t i = 0; i < m_threadCount; i++) {
        taskMsg.type = TaskMsg::NEW_TASK;
        taskMsg.taskCallback = tf;
        taskMsg.args = args;

        ThreadQueue<TaskMsg> *queue = queues[i];
        queue->send(taskMsg);
    }
}