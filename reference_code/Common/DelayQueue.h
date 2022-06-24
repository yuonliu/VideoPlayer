/**
 * @file DelayQueue.hpp
 * @author yilongdong (dyl20001223@163.com)
 * @brief 延迟队列，从pcm绘制音频波形图的中间队列。线程不安全。
 * @version 0.1
 * @date 2022-05-09
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <chrono>
#include <mutex>
#include <queue>

// TODO: 赋值构造函数，用于clear queue = {} 形式
template <typename T>
class DelayQueue {
 public:
  struct Elem {
    T value;
    std::chrono::steady_clock::time_point due_time;  // 到期时间点

    Elem(T val, uint64_t delay_time_ms) : value(val) {
      due_time = std::chrono::steady_clock::now() +
                 std::chrono::milliseconds(delay_time_ms);
    }
    bool IsReady() const { return std::chrono::steady_clock::now() > due_time; }

    struct ElemComp {
      bool operator()(Elem const& left, Elem const& right) const {
        return left.due_time > right.due_time;
      }
    };
  };

 public:
  DelayQueue() = default;
  ~DelayQueue() = default;

  DelayQueue<T>& operator=(DelayQueue<T> other) {
    time_heap_ = other.time_heap_;
    ready_queue_ = other.ready_queue_;
    return *this;
  }

  void push(Elem elem) {
    while (!time_heap_.empty() && time_heap_.top().IsReady()) {
      ready_queue_.push(time_heap_.top().value);
      time_heap_.pop();
    }
    if (elem.IsReady()) {
      ready_queue_.push(elem.value);
    } else {
      time_heap_.push(elem);
    }
  }

  void pop() {
    while (!time_heap_.empty() && time_heap_.top().IsReady()) {
      ready_queue_.push(time_heap_.top().value);
      time_heap_.pop();
    }
    ready_queue_.pop();
  }

  T& front() {
    while (!time_heap_.empty() && time_heap_.top().IsReady()) {
      ready_queue_.push(time_heap_.top().value);
      time_heap_.pop();
    }
    return ready_queue_.front();
  }

  bool empty() {
    while (!time_heap_.empty() && time_heap_.top().IsReady()) {
      ready_queue_.push(time_heap_.top().value);
      time_heap_.pop();
    }
    return ready_queue_.empty();
  }

  uint32_t size() {
    while (!time_heap_.empty() && time_heap_.top().IsReady()) {
      ready_queue_.push(time_heap_.top().value);
      time_heap_.pop();
    }
    return ready_queue_.size();
  }

 private:
  // 考虑通用性，这里用堆来实现。虽然没必要。
  std::priority_queue<Elem, std::vector<Elem>, typename Elem::ElemComp>
      time_heap_;              // 时间小顶堆, 堆排序是不稳定的
  std::queue<T> ready_queue_;  // 已经到期元素的队列
};