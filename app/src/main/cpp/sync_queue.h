//
// Created by chengjunsen on 2018/10/27.
//

#ifndef SPLAYER_SYNC_QUEUE_H
#define SPLAYER_SYNC_QUEUE_H

#include <queue>
#include <cwchar>
#include <pthread.h>

template <class T>
class SyncQueue {
public:
    SyncQueue() {
        pthread_mutex_init(&mutex, NULL);
    }

    ~SyncQueue() {
        pthread_mutex_destroy(&mutex);
    }

    void push(T t) {
        pthread_mutex_lock(&mutex);
        queue.push(t);
        pthread_mutex_unlock(&mutex);
    }

    void push(std::vector<T> vec) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < vec.size(); ++i) {
            queue.push(vec[i]);
        }
        pthread_mutex_unlock(&mutex);
    }

    T pop() {
        pthread_mutex_lock(&mutex);
        T t = queue.front();
        queue.pop();
        pthread_mutex_unlock(&mutex);
        return t;
    }

    bool isEmpty() {
        return queue.empty();
    }

    size_t size() {
        return queue.size();
    }

private:
    pthread_mutex_t mutex;
    std::queue<T> queue;
};

#endif //SPLAYER_SYNC_QUEUE_H
