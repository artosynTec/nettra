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

void ByteBuf::clear() {
    length = head = 0;
}

void ByteBuf::adjust() {
    if(head != 0) {
        if (length != 0) {
            memmove(data, data + head,length);
        }
        
        head = 0;
    }
}

void ByteBuf::copy(const ByteBuf *sourceBuf) {
    memcpy(data,sourceBuf->data + sourceBuf->head,sourceBuf->length);
    head = 0;
    length = sourceBuf->length;
}

void ByteBuf::pop(int len) {
    length -= len;
    head += len;
}

int ByteBuf::writeableBytes() {
    return this->capacity - this->length;
}

int ByteBuf::readableBytes() {
    return this->length - this->head;
}