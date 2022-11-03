#ifndef ANDROID_LIBTRANSCODE_DATABUFFER
#define ANDROID_LIBTRANSCODE_DATABUFFER

#include <Lock.h>
#include <queue>

using namespace std;

struct QueueItem {
    void *data;
    int len;
};


class DataBuffer {
public:
    DataBuffer(int depth);
    ~DataBuffer();
    inline Lock &GetLock() { return mLock; };

protected:
    virtual bool IsFull();
    virtual bool Push(QueueItem *item);
    virtual QueueItem *Pop();
    virtual QueueItem *GetFront();
    virtual int GetQueueDepth();
    virtual int GetQueueMaxDepth();
    virtual QueueItem *CreateItem(int size);
    virtual void DeleteItem(QueueItem *item);
    virtual void EraseQueue();
private:
    int mMaxDepth;
    deque<QueueItem *> mQueue;
    Lock mLock;
};


#endif //ANDROID_LIBTRANSCODE_DATABUFFER
