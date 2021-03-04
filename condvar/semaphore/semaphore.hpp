#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace solutions {

// A Counting semaphore

// Semaphores are often used to restrict the number of threads
// than can access some (physical or logical) resource

class Semaphore {
 public:
  // Creates a Semaphore with the given number of permits
  explicit Semaphore(size_t initial) : n_permits_(initial) {
  }

  // Acquires a permit from this semaphore,
  // blocking until one is available
  void Acquire() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (n_permits_ == 0) {
      waiter_.wait(lock);
    }
    n_permits_ = n_permits_ - 1;
  }

  // Releases a permit, returning it to the semaphore
  void Release() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    n_permits_ = n_permits_ + 1;
    waiter_.notify_one();
  }

 private:
  volatile size_t n_permits_;
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable waiter_;
};

}  // namespace solutions
