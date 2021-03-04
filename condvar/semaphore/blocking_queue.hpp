#pragma once

#include "semaphore.hpp"

#include <deque>
#include <queue>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t capacity) : cur_capacity_(capacity) {
  }

  // Inserts the specified element into this queue
  void Put(T value) {
    cur_capacity_.Acquire();
    mutex_.Acquire();

    data_.push(std::move(value));
    cur_elements_.Release();

    mutex_.Release();
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    cur_elements_.Acquire();
    mutex_.Acquire();

    T value(std::move(data_.front()));
    data_.pop();
    cur_capacity_.Release();

    mutex_.Release();
    return T(std::move(value));
  }

 private:
  std::queue<T> data_;
  Semaphore cur_elements_{0};
  Semaphore cur_capacity_;  // capacity - cur_elements_;
  Semaphore mutex_{1};
};

}  // namespace solutions
