#pragma once
#include "iostream"


class ByteBuf {
public:
    ByteBuf(int size);
    ~ByteBuf();

    void Clear();
    void Adjust();
    void Copy(const ByteBuf *sourceBuf);
    void Pop(int len);
    int WriteableBytes();
    int ReadableBytes();

    ByteBuf *next;
    int capacity;
    int length;
    int head;
    char *data;
};
