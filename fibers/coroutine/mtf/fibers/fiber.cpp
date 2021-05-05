#include <mtf/fibers/api.hpp>

#include <mtf/coroutine/impl.hpp>
#include <mtf/fibers/stacks.hpp>

#include <mtf/fibers/spinlock.hpp>
#include <mutex>

#include <vector>

#include <iostream>

namespace mtf::fibers {

using coroutine::impl::Coroutine;
using tp::StaticThreadPool;

class StackPool {
 public:
  context::Stack* Acquire() {
    std::unique_lock lock(mutex_);

    if (free_stacks_.empty()) {
      lock.unlock();
      context::Stack new_stack = AllocateStack();
      lock.lock();

      stacks_.push_back(std::move(new_stack));
      return &stacks_.back();
    }

    context::Stack* res = free_stacks_.back();
    free_stacks_.pop_back();
    return res;
  }

  void Release(context::Stack* stack) {
    std::lock_guard lock(mutex_);
    free_stacks_.push_back(stack);
  }

 private:
  std::vector<context::Stack> stacks_;
  std::vector<context::Stack*> free_stacks_;
  TASSpinLock mutex_;

} stack_pool;

////////////////////////////////////////////////////////////////////////////////

class Fiber;

static thread_local Fiber* current_fiber = nullptr;

class Fiber {
 public:
  Fiber(Routine routine, StaticThreadPool& scheduler)
      : stack_(stack_pool.Acquire()),
        coroutine_(std::move(routine), stack_->View()),
        scheduler_(scheduler) {
    scheduler_.Submit([this]() {
      Worker();
    });
  }

  void Worker() {
    current_fiber = this;
    coroutine_.Resume();
    current_fiber = nullptr;

    if (!coroutine_.IsCompleted()) {
      scheduler_.Submit([this]() {
        Worker();
      });

    } else {
      scheduler_.SubmitContinuation([this]() {
        stack_pool.Release(stack_);
        delete this;
      });
    }
  }

  static void Yield() {
    Coroutine::Suspend();
  }

 private:
  context::Stack* stack_;
  Coroutine coroutine_;
  StaticThreadPool& scheduler_;
};

////////////////////////////////////////////////////////////////////////////////

void Spawn(Routine routine, StaticThreadPool& scheduler) {
  new Fiber(std::move(routine), scheduler);
}

void Spawn(Routine routine) {
  if (StaticThreadPool::Current() == nullptr) {
    return;  // user fault
  }

  new Fiber(std::move(routine), *StaticThreadPool::Current());
}

void Yield() {
  Fiber::Yield();
}

}  // namespace mtf::fibers
