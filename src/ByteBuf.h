#pragma once
#include "iostream"


class ByteBuf {
public:
    ByteBuf(int size);
    ~ByteBuf();

    void clear();
    void adjust();
    void copy(const ByteBuf *sourceBuf);
    void pop(int len);
    int writeableBytes();
    int readableBytes();

    ByteBuf *next;
    int capacity;
    int length;
    int head;
    char *data;
};
