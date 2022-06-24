//
// Created by 于承业 on 2022/4/10.
//

#ifndef GLMP_RINGQUEUE_HPP
#define GLMP_RINGQUEUE_HPP
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
 * 存储的是*TyData，也就是指向对象的指针; 因此为保证正确性，请尽可能在堆上创建对象，并检查是否enqueue同一指针。
 * @tparam TyData 数据类型
 * @tparam _uiCount 缓冲区大小
 */
template < typename TyData, long _uiCount = 100000 >
class lfringqueue
{
public:
    using value_type = TyData;
    explicit lfringqueue( long uiCount = _uiCount ) : m_lTailIterator(0), m_lHeadIterator(0), m_uiCount( uiCount )
    {
        m_queue = new TyData*[m_uiCount]{nullptr};
//        memset(m_queue, 0, sizeof(TyData*) * m_uiCount );
    }

    ~lfringqueue(){
        delete [] m_queue;
    }

    bool enqueue( TyData *pdata, unsigned int uiRetries = 1000 )
    {
        if (nullptr==pdata){
            // Null enqueues are not allowed
            return false;
        }
        unsigned int uiCurrRetries = 0;
        while ( uiCurrRetries < uiRetries )
        {
            // Release fence in order to prevent memory reordering
            // of any read or write with following write
            std::atomic_thread_fence(std::memory_order_release);

            long lHeadIterator = m_lHeadIterator;

            if (nullptr == m_queue[lHeadIterator] )
            {
                long lHeadIteratorOrig = lHeadIterator;

                ++lHeadIterator;
                if ( lHeadIterator >= m_uiCount )
                    lHeadIterator = 0;

                // Don't worry if this CAS fails.  It only means some thread else has
                // already inserted an item and set it.
                if ( std::atomic_compare_exchange_strong( &m_lHeadIterator, &lHeadIteratorOrig, lHeadIterator ) )
                {
                    // void* are always atomic (you wont set a partial pointer).
                    m_queue[lHeadIteratorOrig] = pdata;

                    if ( m_lEventSet.test_and_set( ))
                    {
                        m_bHasItem.test_and_set();
                    }
                    return true;
                }
            }
            else
            {
                // The queue is full.  Spin a few times to check to see if an item is popped off.
                ++uiCurrRetries;
            }
        }
        return false;
    }

    bool dequeue( TyData **ppdata )
    {
        if ( !ppdata )
        {
            // Null dequeues are not allowed!
            return false;
        }

        bool bDone = false;
        bool bCheckQueue = true;

        while ( !bDone )
        {
            // Acquire fence in order to prevent memory reordering
            // of any read or write with following read
            std::atomic_thread_fence(std::memory_order_acquire);
            //MemoryBarrier();
            long lTailIterator = m_lTailIterator;
            TyData *pdata = m_queue[lTailIterator];
            //volatile _TyData *pdata = m_queue[lTailIterator];
            if (nullptr != pdata )
            {
                bCheckQueue = true;
                long lTailIteratorOrig = lTailIterator;

                ++lTailIterator;
                if ( lTailIterator >= m_uiCount )
                    lTailIterator = 0;

                //if ( lTailIteratorOrig == atomic_cas( (volatile long*)&m_lTailIterator, lTailIterator, lTailIteratorOrig ))
                if ( std::atomic_compare_exchange_strong( &m_lTailIterator, &lTailIteratorOrig, lTailIterator ))
                {
                    // Sets of sizeof(void*) are always atomic (you wont set a partial pointer).
                    m_queue[lTailIteratorOrig] = nullptr;

                    // Gets of sizeof(void*) are always atomic (you wont get a partial pointer).
                    *ppdata = (TyData*)pdata;

                    return true;
                }
            }
            else
            {
                bDone = true;
                m_lEventSet.clear();
            }
        }
        *ppdata = nullptr;
        return false;
    }


    long countguess() const
    {
        long lCount = trycount();

        if ( 0 != lCount )
            return lCount;

        // If the queue is full then the item right before the tail item will be valid.  If it
        // is empty then the item should be set to NULL.
        long lLastInsert = m_lTailIterator - 1;
        if ( lLastInsert < 0 )
            lLastInsert = m_uiCount - 1;

        TyData *pdata = m_queue[lLastInsert];
        if ( pdata != nullptr )
            return m_uiCount;

        return 0;
    }

    long getmaxsize() const
    {
        return m_uiCount;
    }

    bool HasItem()
    {
        return m_bHasItem.test_and_set();
    }

    void SetItemFlagBack()
    {
        m_bHasItem.clear();
    }

private:
    long trycount() const
    {
        long lHeadIterator = m_lHeadIterator;
        long lTailIterator = m_lTailIterator;

        if ( lTailIterator > lHeadIterator )
            return m_uiCount - lTailIterator + lHeadIterator;

        // This has a bug where it returns 0 if the queue is full.
        return lHeadIterator - lTailIterator;
    }

private:
    std::atomic<long> m_lHeadIterator;  // enqueue index
    std::atomic<long> m_lTailIterator;  // dequeue index
    TyData **m_queue;                  // array of pointers to the data
    long m_uiCount;                     // size of the array
    std::atomic_flag m_lEventSet{ATOMIC_FLAG_INIT};       // a flag to use whether we should change the item flag
    std::atomic_flag m_bHasItem{ATOMIC_FLAG_INIT};        // a flag to indicate whether there is an item enqueued
};

#endif //GLMP_RINGQUEUE_HPP
