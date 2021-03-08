#pragma once

#include <tp/blocking_queue.hpp>
#include <tp/task.hpp>

#include <twist/stdlike/thread.hpp>
#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/atomic.hpp>
#include <twist/util/thread_local.hpp>

namespace tp {

// Fixed-size pool of worker threads

class StaticThreadPool {
 public:
  explicit StaticThreadPool(size_t n_workers);
  ~StaticThreadPool();

  // Non-copyable
  StaticThreadPool(const StaticThreadPool&) = delete;
  StaticThreadPool& operator=(const StaticThreadPool&) = delete;

  // Schedules task for execution in one of the worker threads
  void Submit(Task task);

  // Graceful shutdown
  // Waits until outstanding work count has reached 0
  // and joins worker threads
  void Join();

  // Hard shutdown
  // Joins worker threads ASAP
  void Shutdown();

  // Locate current thread pool from worker thread
  static StaticThreadPool* Current();

 private:
  void InitWorkers(size_t n_workers);
  void Worker();
  void JoinWorkers();

  std::vector<std::thread> workers_;
  UnboundedBlockingQueue<Task> tasks_;
  twist::stdlike::atomic<uint32_t> tasks_pending_{0};

  twist::stdlike::atomic<uint32_t> joined_{0};
  bool closed_ = false;  // joined or shutdowned
};

inline StaticThreadPool* Current() {
  return StaticThreadPool::Current();
}

}  // namespace tp
