#include <cstdlib>
#include <debug.h>
#include "DataBuffer.h"

#define QUEUE_MAX_DEPTH 3


DataBuffer::DataBuffer(int depth) {
    mMaxDepth = depth > 0 ? depth : QUEUE_MAX_DEPTH;
}

DataBuffer::~DataBuffer() {
    EraseQueue();
}

bool DataBuffer::IsFull() {
    AutoLock lock(mLock);
    if(mQueue.empty()){
        return false;
    }
    return mQueue.size()==mMaxDepth;
}

bool DataBuffer::Push(QueueItem *item) {
    if (NULL == item) {
        return false;
    }
    AutoLock lock(mLock);
    mQueue.push_back(item);
    return true;
}

QueueItem *DataBuffer::Pop() {
    AutoLock lock(mLock);
    if (mQueue.empty()) {
        return NULL;
    }
    QueueItem *item = NULL;
    item = mQueue.front();
    if (item) {
        DeleteItem(item);
        mQueue.pop_front();
    }
    return item;
}

QueueItem *DataBuffer::GetFront() {
    AutoLock lock(mLock);
    if (mQueue.empty()) {
        return NULL;
    }
    return mQueue.front();
}


int DataBuffer::GetQueueDepth() {
    AutoLock lock(mLock);
    int ret = 0;
    ret = mQueue.size();
    return ret;
}

int DataBuffer::GetQueueMaxDepth() {
    AutoLock lock(mLock);
    int ret = 0;
    ret = mMaxDepth;
    return ret;
}

void DataBuffer::EraseQueue() {
    QueueItem *item, *next;
    AutoLock lock(mLock);
    int size = mQueue.size();
    while (!mQueue.empty()) {
        item = mQueue.front();
        if (NULL != item) {
            DeleteItem(item);
            mQueue.pop_front();
        }
    }
}

QueueItem *DataBuffer::CreateItem(int size) {
    QueueItem *item = (QueueItem *) calloc(1, size);
    if (NULL == item) {
        return NULL;
    }
    return item;
}

void DataBuffer::DeleteItem(QueueItem *item) {
    AutoLock lock(mLock);
    if (NULL == item)
        return;
    if (item->data) {
        free(item->data);
        item->data = NULL;
    }
    free(item);
    item = NULL;
}
