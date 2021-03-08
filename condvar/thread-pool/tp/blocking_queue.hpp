#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <optional>

#include <queue>

namespace tp {

// Unbounded blocking multi-producers/multi-consumers queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);

    if (!closed_) {
      data_.emplace(std::move(value));
      wait_nempty_.notify_all();
      return true;
    } else {
      return false;
    }
  }

  std::optional<T> Take() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);

    while (data_.empty()) {
      if (closed_) {
        return std::nullopt;
      }
      wait_nempty_.wait(lock);
    }

    T value(std::move(data_.front()));
    data_.pop();
    return std::optional<T>(std::move(value));
  }

  void Close() {
    CloseImpl(/*clear=*/false);
  }

  void Cancel() {
    CloseImpl(/*clear=*/true);
  }

 private:
  void CloseImpl(bool clear) {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);

    if (clear) {
      data_ = std::queue<T>();
    }

    closed_ = true;
    wait_nempty_.notify_all();
  }

 private:
  std::queue<T> data_;
  bool closed_ = false;

  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable wait_nempty_;
};

}  // namespace tp
