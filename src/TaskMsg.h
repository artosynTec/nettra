#pragma once

#include "EventLoop.h"

typedef void (*taskFun)(EventLoop *loop,void *args);

struct TaskMsg {
    enum TASK_TYPE {
        NEW_CONN,
        NEW_TASK,
    };

    TASK_TYPE type;

    union {
        int connfd;
        struct {
            taskFun taskCallback;
            void *args;
        };  
    };
};
