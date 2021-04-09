#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

#include "wait_queue.hpp"

#include <cstdlib>

namespace solutions {

using twist::util::SpinWait;

class Mutex {
 public:
  void Lock() {
    n_threads_.fetch_add(1);
    while (locked_.exchange(1) == 1) {  // while was locked_
      locked_.wait(1);                  // wait until unlocked notification
    }
  }

  void Unlock() {
    locked_.store(0);  // unlock
    if (n_threads_.fetch_sub(1) != 1) {
      locked_.notify_one();  // unlock notification
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> locked_{0};  // bool is not supported
  twist::stdlike::atomic<uint32_t> n_threads_{0};
};

}  // namespace solutions
