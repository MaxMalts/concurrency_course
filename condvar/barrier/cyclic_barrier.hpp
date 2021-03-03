#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>
#include <iostream>

namespace solutions {

// CyclicBarrier allows a set of threads to all wait for each other
// to reach a common barrier point

// The barrier is called cyclic because
// it can be re-used after the waiting threads are released.

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t n_participants)
      : n_participants_(n_participants) {
  }

  // Blocks until all participants have invoked Arrive()
  void Arrive() {
    //    twist::stdlike::mutex inner_mutex;
    //    mutex_.lock();
    //    cur_participants_ = cur_participants_ + 1;
    //    if (cur_participants_ < n_participants_) {
    //      mutex_.unlock();
    //
    //      while (!all_arrived_) {
    //      }
    //      inner_mutex.lock();
    //      cur_participants_ = cur_participants_ - 1;
    //      inner_mutex.unlock();
    //      return;
    //    }
    //
    //    all_arrived_ = true;
    //    while (cur_participants_ > 1) {
    //      // wait until all threads are continued
    //    }
    //    inner_mutex.lock();
    //    cur_participants_ = cur_participants_ - 1;
    //    inner_mutex.unlock();
    //    all_arrived_ = false;
    //
    //    mutex_.unlock();

    std::unique_lock<twist::stdlike::mutex> lock(mutex_);

    // Handling group that "overlayed" previous group
    while (all_arrived_) {
      // wait until previous arrive handling finishes
      overlay_waiter_.wait(lock, [this] {
        return !all_arrived_;
      });  // (*)

      overlay_waiter_.notify_one();  // needed to do this due to TL
    }

    cur_participants_ = cur_participants_ + 1;

    // Handling not last thread in current group
    if (cur_participants_ < n_participants_) {
      while (!all_arrived_) {
        middle_waiter_.wait(lock, [this] {
          return all_arrived_;
        });
      }

      cur_participants_ = cur_participants_ - 1;
      last_waiter_.notify_one();  // for last thread in this group (**)
      return;
    }

    // Handling last thread in current group
    all_arrived_ = true;
    middle_waiter_.notify_all();
    while (cur_participants_ > 1) {
      // wait until all threads are continued
      last_waiter_.wait(lock, [this] {
        return cur_participants_ == 1;
      });  // (**)
    }
    cur_participants_ = cur_participants_ - 1;
    all_arrived_ = false;
    overlay_waiter_.notify_one();  // for next group of threads (*)
  }

 private:
  size_t n_participants_ = 0;
  volatile size_t cur_participants_ = 0;
  volatile bool all_arrived_ = false;

  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable middle_waiter_;
  twist::stdlike::condition_variable overlay_waiter_;
  twist::stdlike::condition_variable last_waiter_;
};

}  // namespace solutions
