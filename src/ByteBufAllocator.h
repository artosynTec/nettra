#pragma once

#include "ByteBuf.h"
#include <ext/hash_map>

using pool_t = __gnu_cxx::hash_map<int, ByteBuf *>;
#define MAX_POOL_LIMIT (1024*1024)

enum MEM_CAP {
    M1K = 1024,
    M2K = 2048,
    M4K = 4096,
    M16K = 16384
};

class ByteBufAllocator {
private:
    ByteBufAllocator();
    ByteBufAllocator(const ByteBufAllocator &);
    const ByteBufAllocator &operator=(const ByteBufAllocator &);
    uint64_t m_totalMem;
    pool_t m_pool;
    static ByteBufAllocator *m_instance;
    static pthread_once_t m_once;
    static pthread_mutex_t m_mutex;
public:
    static void init() {
        m_instance = new ByteBufAllocator();
    }

    static ByteBufAllocator *instance() {
        pthread_once(&m_once,init);
        return m_instance;
    }

    ByteBuf *AllocBuf(int size);
    
    ByteBuf *AllocBuf() {
        return AllocBuf(M1K);
    }

    void Release(ByteBuf *byteBuf);
};
