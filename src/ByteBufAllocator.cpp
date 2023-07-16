#include "ByteBufAllocator.h"

ByteBufAllocator *ByteBufAllocator::m_instance = nullptr;
pthread_once_t ByteBufAllocator::m_once = PTHREAD_ONCE_INIT;
pthread_mutex_t ByteBufAllocator::m_mutex = PTHREAD_MUTEX_INITIALIZER;

ByteBufAllocator::ByteBufAllocator() : m_totalMem(0) {
    ByteBuf *prev;

    // 1k * 100
    m_pool[M1K] = new ByteBuf(M1K);
    if (m_pool[M1K] == nullptr) {
        fprintf(stderr,"alloc 1k ByteBuf error");
        exit(1);
    }

    prev = m_pool[M1K];
    for (size_t i = 0; i < 100; i++) {
        prev->next = new ByteBuf(M1K);
        prev = prev->next;
    }
    
    m_totalMem += 1*100;

    // 2k * 100

    m_pool[M2K] = new ByteBuf(M2K);
    if (m_pool[M2K] == nullptr) {
        fprintf(stderr,"alloc 1k ByteBuf error");
        exit(1);
    }

    prev = m_pool[M2K];
    for (size_t i = 0; i < 100; i++) {
        prev->next = new ByteBuf(M2K);
        prev = prev->next;
    }

    m_totalMem += 2*100;


    // 4k * 50
    m_pool[M4K] = new ByteBuf(M4K);
    if (m_pool[M4K] == nullptr) {
        fprintf(stderr,"alloc 4k ByteBuf error");
        exit(1);
    }
    
    prev = m_pool[M4K];

    for (size_t i = 0; i < 50; i++) {
        prev->next = new ByteBuf(M4K);
        prev = prev->next;
    }
    
    m_totalMem += 4*50;

    // 16k * 10
    m_pool[M16K] = new ByteBuf(M16K);
    if (m_pool[M16K] == nullptr) {
        fprintf(stderr,"alloc 4k ByteBuf error");
        exit(1);
    }

    prev = m_pool[M16K];

    for (size_t i = 0; i < 10; i++) {
        prev->next = new ByteBuf(M16K);
        prev = prev->next;
    }

    m_totalMem += 16*10;
}

ByteBuf *ByteBufAllocator::allocBuf(int size) {
    int index;

    if(size <= M1K) {
        index = M1K;
    } else if (size <= M2K) {
        index = M2K;
    } else if (size <= M4K) {
        index = M4K;
    } else if (size <= M16K) {
        index = M16K;
    }
    
    
    

    // TODO 如果这里是空的，那么不应该这么处理，返回一个非池buf没有意义
    if (m_pool[index] == nullptr) {
        if (m_totalMem + index / 1024 >= MAX_POOL_LIMIT) {
            fprintf(stderr,"memory exceed");
            return nullptr;
        }
        
        ByteBuf *byteBuf = new ByteBuf(index);
        m_totalMem += index / 1024;

        return byteBuf;
    }
    
    ByteBuf *head = m_pool[index];
    m_pool[index] = head->next;
    head->next = nullptr;

    return head;
}

void ByteBufAllocator::release(ByteBuf *byteBuf) {
    int index = byteBuf->capacity;
    byteBuf->length = 0;
    byteBuf->head = 0;

    pthread_mutex_lock(&m_mutex);

    byteBuf->next = m_pool[index];
    m_pool[index] = byteBuf;

    pthread_mutex_unlock(&m_mutex);
}