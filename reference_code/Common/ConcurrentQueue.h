/**
 * @author yilongdong
 * @date 2022-04-27
 * @brief 并发队列模板，有timeout, wait, try模式
 * @details
 * 有两个模板参数，一个是队列中的元素类型，一个队列容器类型。要求队列容器有STL
 * std::queue的类似接口
 * TODO: 改成无锁模式
 */
#pragma once
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <numeric>
#include <queue>

#include "Log/Log.h"

template <typename T, typename QueueCon = std::queue<T>>
class ConcurrentQueue {
 public:
  using mutex_type = std::mutex;
  using value_type = T;

 public:
  ConcurrentQueue(uint32_t capacity);
  ~ConcurrentQueue() = default;

  void push(T const& value);
  void pop(T& value);

  bool push_timeout(T const& value, uint32_t timeout);
  bool pop_timeout(T& value, uint32_t timeout);

  // TODO: 设置超时时间
  bool try_pop(T& value);
  bool try_push(T const& value);

  uint32_t size() const;
  uint32_t capacity() const;
  void clear();

 private:
  QueueCon unsafe_queue_;
  uint32_t capacity_;
  mutable mutex_type mutex_;
  std::condition_variable cv_empty_;
  std::condition_variable cv_full_;
};

template <typename T, typename QueueCon>
ConcurrentQueue<T, QueueCon>::ConcurrentQueue(uint32_t capacity)
    : capacity_(capacity){};

template <typename T, typename QueueCon>
void ConcurrentQueue<T, QueueCon>::push(T const& value) {
  std::unique_lock<mutex_type> locker(mutex_);
  cv_full_.wait(locker, [this] { return unsafe_queue_.size() < capacity_; });
  unsafe_queue_.push(value);
  cv_empty_.notify_one();
}
template <typename T, typename QueueCon>
void ConcurrentQueue<T, QueueCon>::pop(T& value) {
  std::unique_lock<mutex_type> locker(mutex_);
  cv_empty_.wait(locker, [this] { return !unsafe_queue_.empty(); });
  value = std::move(unsafe_queue_.front());
  unsafe_queue_.pop();
  cv_full_.notify_one();
}
template <typename T, typename QueueCon>
bool ConcurrentQueue<T, QueueCon>::try_pop(T& value) {
  std::unique_lock<mutex_type> locker(mutex_);
  if (unsafe_queue_.empty()) return false;
  value = std::move(unsafe_queue_.front());
  unsafe_queue_.pop();
  cv_full_.notify_all();
  return true;
}

template <typename T, typename QueueCon>
bool ConcurrentQueue<T, QueueCon>::try_push(T const& value) {
  std::unique_lock<mutex_type> locker(mutex_);
  if (unsafe_queue_.size() == capacity_) return false;
  unsafe_queue_.push(value);
  cv_empty_.notify_all();
  return true;
}

template <typename T, typename QueueCon>
uint32_t ConcurrentQueue<T, QueueCon>::size() const {
  std::unique_lock<mutex_type> locker(mutex_);
  return static_cast<uint32_t>(unsafe_queue_.size());
}

template <typename T, typename QueueCon>
uint32_t ConcurrentQueue<T, QueueCon>::capacity() const {
  return capacity_;
}

template <typename T, typename QueueCon>
void ConcurrentQueue<T, QueueCon>::clear() {
  std::unique_lock<mutex_type> locker(mutex_);
  unsafe_queue_ = {};
}

template <typename T, typename QueueCon>
bool ConcurrentQueue<T, QueueCon>::push_timeout(T const& value,
                                                uint32_t timeout) {
  std::unique_lock<mutex_type> locker(mutex_);
  if (cv_full_.wait_for(locker, std::chrono::milliseconds(timeout),
                        [this] { return unsafe_queue_.size() < capacity_; })) {
    unsafe_queue_.push(value);
    cv_empty_.notify_all();
    return true;
  };
  return false;
}

template <typename T, typename QueueCon>
bool ConcurrentQueue<T, QueueCon>::pop_timeout(T& value, uint32_t timeout) {
  std::unique_lock<mutex_type> locker(mutex_);
  if (cv_empty_.wait_for(locker, std::chrono::milliseconds(timeout),
                         [this] { return !unsafe_queue_.empty(); })) {
    value = unsafe_queue_.front();
    unsafe_queue_.pop();
    cv_full_.notify_all();
    return true;
  };
  return false;
}
