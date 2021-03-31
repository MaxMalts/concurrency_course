#include <tp/static_thread_pool.hpp>

#include <tp/helpers.hpp>
#include <wheels/test/test_framework.hpp>

#include <twist/util/thread_local.hpp>

#include <assert.h>

namespace tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocal<StaticThreadPool*> pool{nullptr};

////////////////////////////////////////////////////////////////////////////////

StaticThreadPool::StaticThreadPool(size_t n_workers) {
  InitWorkers(n_workers);
}

StaticThreadPool::~StaticThreadPool() {
  assert(closed_);
}

void StaticThreadPool::Submit(Task task) {
  tasks_pending_.fetch_add(1);
  tasks_.Put(Task(std::move(task)));
}

void StaticThreadPool::Join() {
  joined_.store(1);
  if (tasks_pending_.load() == 0) {
    tasks_.Close();
  }
  JoinWorkers();
}

void StaticThreadPool::Shutdown() {
  tasks_.Cancel();
  JoinWorkers();
}

void StaticThreadPool::JoinWorkers() {
  for (twist::stdlike::thread& worker : workers_) {
    worker.join();
  }
  closed_ = true;
}

StaticThreadPool* StaticThreadPool::Current() {
  return *pool;
}

void StaticThreadPool::InitWorkers(size_t n_workers) {
  for (size_t i = 0; i < n_workers; ++i) {
    workers_.emplace_back([this] {
      *pool = this;
      Worker();
    });
  }
}

void StaticThreadPool::Worker() {
  std::optional<Task> cur_task = tasks_.Take();

  while (cur_task != std::nullopt) {
    ExecuteHere(cur_task.value());
    if (tasks_pending_.fetch_sub(1) == 1 && joined_.load() == 1) {
      tasks_.Close();
    }

    cur_task = tasks_.Take();
  }
}

}  // namespace tp
