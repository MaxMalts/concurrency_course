#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdint>

namespace solutions {

class ConditionVariable {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& mutex) {
    ++n_waiters_;

    mutex.unlock();
    waiter_block_.wait(1);
    mutex.lock();

    --n_waiters_;
    if (n_waiters_ == 0) {
      waiter_block_.store(1);
    }
  }

  void NotifyOne() {
    waiter_block_.store(0);
    waiter_block_.notify_one();
  }

  void NotifyAll() {
    waiter_block_.store(0);
    waiter_block_.notify_all();
  }

 private:
  twist::stdlike::atomic<uint32_t> waiter_block_{0};
  size_t n_waiters_ = 0;
};

}  // namespace solutions
