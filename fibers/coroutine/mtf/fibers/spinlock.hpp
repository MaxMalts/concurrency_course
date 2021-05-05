#pragma once

#include <wheels/support/cpu.hpp>
#include <twist/stdlike/atomic.hpp>

namespace mtf {

class TASSpinLock {
 public:
  void Lock() {
    while (locked_.exchange(1) != 0) {
      while (locked_.load()) {
        wheels::SpinLockPause();
      }
    }
  }

  void lock() {
    Lock();
  }

  bool TryLock() {
    return locked_.exchange(1) == 0;
  }

  void Unlock() {
    locked_.store(0);
  }

  void unlock() {
    Unlock();
  }

 private:
  twist::stdlike::atomic<uint64_t> locked_{0};
};

}  // namespace mtf
