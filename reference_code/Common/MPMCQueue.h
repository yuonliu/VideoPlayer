//
// Created by ByteDance on 2022/5/21.
//

#ifndef QTMEDIAPLAYER_MPMCQUEUE_H
#define QTMEDIAPLAYER_MPMCQUEUE_H
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>

// Lock free ring queue

/**
 * 无锁环形队列，支持wait, try, timeout
 * @tparam T 数据类型
 * @tparam defaultCapacity 缓冲区大小
 */
template < typename T, long defaultCapacity = 100000 >
class lfringqueue {
public:
    using value_type = T;
    explicit lfringqueue(std::size_t capacity_ = defaultCapacity) : tail_i(0), head_i(0), capacity(capacity_ ) {
        queue = new T*[capacity]{nullptr};
    }

    ~lfringqueue(){
        delete[] queue;
    }

    template<typename ...Args>
    bool try_emplace(std::size_t maxRetry, Args&&... args){
        return emplace(maxRetry, std::forward<Args>(args)...);
    }

    bool try_push(const T& elem, std::size_t maxRetry = 1000 ) {
        return push(elem, maxRetry);
    }

    bool try_pop(T& elem, std::size_t maxRetry = 1000) {
        std::size_t trials = 0;
        while(trials < maxRetry){
            if(pop(elem)){
                return true;
            } else ++trials;
        }
    }

    template<typename ...Args>
    bool wait_emplace(Args&&... args){
        return emplace(0, std::forward<Args>(args)...);
    }

    void wait_push(const T& elem) {
        push(elem, 0);
    }

    void wait_pop(T& elem) {
        for(;;){
            if(pop(elem)){
                return;
            }
        }
    }

    template<typename ...Args>
    bool emplace_timeout(std::size_t timeout, Args&&... args){
        std::mutex t_mtx;
        std::unique_lock<std::mutex> lk(t_mtx);
        if(cv_full.wait_for(lk, std::chrono::milliseconds(timeout),[this]{return countguess()<capacity;})){
            emplace(std::forward<Args>(args)...);
            return true;
        }
        return false;
    }

    bool push_timeout(const T& elem, std::size_t timeout) {
        std::mutex t_mtx;
        std::unique_lock<std::mutex> lk(t_mtx);
        if(cv_full.wait_for(lk, std::chrono::milliseconds(timeout),[this]{return countguess()<capacity;})){
            push(elem);
            return true;
        }
        return false;
    }

    bool pop_timeout(T& elem, std::size_t timeout) {
        std::mutex t_mtx;
        std::unique_lock<std::mutex> lk(t_mtx);
        if(cv_empty.wait_for(lk, std::chrono::milliseconds(timeout),[this]{return countguess()>0;})){
            return try_pop(elem, std::min(capacity-countguess(), 100));;
        }
        return false;
    }

    long countguess() const
    {
        long lCount = trycount();

        if ( 0 != lCount )
            return lCount;

        long lLastInsert = tail_i - 1;
        if ( lLastInsert < 0 )
            lLastInsert = capacity - 1;

        T *pdata = queue[lLastInsert];
        if ( pdata != nullptr )
            return capacity;

        return 0;
    }

    long getmaxsize() const { return capacity; }

    bool HasItem() { return hasItem.test_and_set(); }

    void SetItemFlagBack() { hasItem.clear(); }

private:
    template<typename ...Args>
    bool emplace(std::size_t maxRetry, Args&&... args){
        auto* newNode = new T(std::forward<Args>(args)...);
        std::size_t trials = 0;
        while (trials < maxRetry )
        {
            std::atomic_thread_fence(std::memory_order_release);
            long lHeadIterator = head_i;
            if (nullptr == queue[lHeadIterator] ) {
                long lHeadIteratorOrig = lHeadIterator;
                ++lHeadIterator;
                if (lHeadIterator >= capacity )
                    lHeadIterator = 0;

                if ( std::atomic_compare_exchange_strong(&head_i, &lHeadIteratorOrig, lHeadIterator ) )
                {
                    queue[lHeadIteratorOrig] = newNode;
                    if ( eventSet.test_and_set()) {
                        hasItem.test_and_set();
                    }
                    return true;
                }
            }
            else {
                trials = (maxRetry>0)?trials+1:0;
            }
        }
        delete newNode;
        return false;
    }

    bool push(const T& elem, std::size_t maxRetry = 1000 )
    {
        auto* newNode = new T(elem);
        std::size_t trials = 0;
        while (trials < maxRetry) {
            std::atomic_thread_fence(std::memory_order_release);

            long lHeadIterator = head_i;

            if (nullptr == queue[lHeadIterator] )
            {
                long lHeadIteratorOrig = lHeadIterator;

                ++lHeadIterator;
                if (lHeadIterator >= capacity )
                    lHeadIterator = 0;

                if ( std::atomic_compare_exchange_strong(&head_i, &lHeadIteratorOrig, lHeadIterator ) )
                {
                    queue[lHeadIteratorOrig] = newNode;

                    if ( eventSet.test_and_set()) {
                        hasItem.test_and_set();
                    }
                    return true;
                }
            }
            else {
                trials = (maxRetry>0)?trials+1:0;
            }
        }
        delete newNode;
        return false;
    }

    bool pop(T& elem) {
        bool bDone = false;
        bool bCheckQueue = true;

        while (!bDone) {
            std::atomic_thread_fence(std::memory_order_acquire);
            long lTailIterator = tail_i;
            T *p = queue[lTailIterator];
            if (nullptr!=p) {
                bCheckQueue = true;
                long lTailIteratorOrig = lTailIterator;

                ++lTailIterator;
                if (lTailIterator >= capacity ) lTailIterator = 0;

                if ( std::atomic_compare_exchange_strong(&tail_i, &lTailIteratorOrig, lTailIterator )) {
                    queue[lTailIteratorOrig] = nullptr;
                    elem = *p;
                    return true;
                }
            }
            else {
                bDone = true;
                eventSet.clear();
            }
        }
        return false;
    }

    long trycount() const {
        long lHeadIterator = head_i;
        long lTailIterator = tail_i;

        if ( lTailIterator > lHeadIterator )
            return capacity - lTailIterator + lHeadIterator;

        return lHeadIterator - lTailIterator;
    }

private:
    std::atomic<long> head_i;
    std::atomic<long> tail_i;
    T **queue;
    std::size_t capacity;
    std::atomic_flag eventSet{ATOMIC_FLAG_INIT};       // a flag to use whether we should change the item flag
    std::atomic_flag hasItem{ATOMIC_FLAG_INIT};        // a flag to indicate whether there is an item enqueued
    std::condition_variable cv_full;
    std::condition_variable cv_empty;
};


#endif //QTMEDIAPLAYER_MPMCQUEUE_H
