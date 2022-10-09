//
// Created by HelyskiTank on 2021/11/24.
//

#ifndef ANDROID_LIBTRANSCODE_RINGQUEUE_H
#define ANDROID_LIBTRANSCODE_RINGQUEUE_H

#include <iostream>
#include <stdlib.h>
#include "debug.h"
#include "Lock.h"

#define MAX_RING_BUFFER_SIZE 4
using namespace std;
template <typename T>
class RingQueue
{
private:
    long head, rear;
    int capacity ;
    T *data;
    Lock ringLock;

public:
    // 默认构造函数
    RingQueue();
    // 自定义构造函数
    RingQueue(const int capacity);
    // 析构函数
    ~RingQueue();
    // 拷贝构造函数
    RingQueue(const RingQueue &);
    // 移动构造函数
    RingQueue(RingQueue &&);

    T pop();
    void push(T &e);
    void clearAll();
    int getSize() ;
    int getCapacity() ;
    T getHeadElement() ;
    T getRearElement() ;
    bool isEmpty() ;
    bool isFull() ;

    inline Lock& GetLock(){return ringLock;};
};

template<typename T>
RingQueue<T>::RingQueue() {
    head = rear = 0;
    capacity = MAX_RING_BUFFER_SIZE;
    data = new T[capacity];
}

template<typename T>
RingQueue<T>::RingQueue(int cap) {
    head = rear = 0;
    this->capacity = cap;
    data = new T[capacity];
}

template<typename T>
RingQueue<T>::~RingQueue() {
    delete[] data;
}

template<typename T>
void RingQueue<T>::clearAll() {
    delete[] data;
    data = NULL;
    data = new T[capacity];
}

template<typename T>
int RingQueue<T>::getSize() {
    return (rear + capacity - head) % capacity;
}

template<typename T>
int RingQueue<T>::getCapacity()  {
    return capacity;
}

template<typename T>
bool RingQueue<T>::isFull() {
    return (rear + 1) % capacity == head;
}

template<typename T>
bool RingQueue<T>::isEmpty() {
    return head == rear;
}

/**
 * 默认满队时需要先将队头元素出队，然后新元素入队
 * @tparam T
 * @param e
 */
template<typename T>
void RingQueue<T>::push(T &e) {
    LOGD("yuvRingQueue push...");
    if (isFull()) {
        LOGD("yuvRingQueue is full");
        pop();
    }
    LOGD("yuvRingQueue push to array:%d",rear);
    data[rear] = e;
    rear = (rear + 1) % capacity;
    LOGD("yuvRingQueue headP:%d,update rear to:%d",head,rear);
}

template<typename T>
T RingQueue<T>::pop() {
    LOGD("yuvRingQueue pop...");
    T ret={0};
    if (!isEmpty()) {
        ret = data[head];
        head = (head+1)%capacity;
        LOGD("yuvRingQueue pop end ,update head to:%d",head);
        return ret;
    }else{
        LOGD("yuvRingQueue is empty");
    }
    return ret;
}

template<typename T>
T RingQueue<T>::getHeadElement()  {
    T ret={0};
    if(!isEmpty()) {
        ret = data[head];
    }
    return ret;
}

template<typename T>
T RingQueue<T>::getRearElement()  {
    if(!isEmpty()) {
        return data[rear];
    }
    return NULL;
}


#endif //ANDROID_LIBTRANSCODE_RINGQUEUE_H
