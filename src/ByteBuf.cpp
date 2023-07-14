#include "ByteBuf.h"
#include <assert.h>
#include "stdio.h"
#include <string.h>

ByteBuf::ByteBuf(int size) {
    capacity = size;
    length = 0;
    head = 0;
    next = nullptr;
    data = new char[size];
    
    assert(data);
}

ByteBuf::~ByteBuf() {
    delete data;
    next = nullptr;
}

void ByteBuf::Clear() {
    length = head = 0;
}

void ByteBuf::Adjust() {
    if(head != 0) {
        if (length != 0) {
            memmove(data, data + head,length);
        }
        
        head = 0;
    }
}

void ByteBuf::Copy(const ByteBuf *sourceBuf) {
    memcpy(data,sourceBuf->data + sourceBuf->head,sourceBuf->length);
    head = 0;
    length = sourceBuf->length;
}

void ByteBuf::Pop(int len) {
    length -= len;
    head += len;
}

int ByteBuf::WriteableBytes() {
    return this->capacity - this->length;
}

int ByteBuf::ReadableBytes() {
    return this->length - this->head;
}