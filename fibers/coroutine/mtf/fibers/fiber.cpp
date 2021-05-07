#include <mtf/fibers/api.hpp>

#include <mtf/coroutine/impl.hpp>
#include <mtf/fibers/stacks.hpp>

namespace mtf::fibers {

using coroutine::impl::Coroutine;
using tp::StaticThreadPool;

thread_local StackPool stack_pool;

////////////////////////////////////////////////////////////////////////////////

class Fiber;

static thread_local Fiber* current_fiber = nullptr;

class Fiber {
 public:
  Fiber(Routine routine, StaticThreadPool& scheduler)
      : stack_(stack_pool.Acquire()),
        coroutine_(std::move(routine), stack_.View()),
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
      stack_pool.Release(std::move(stack_));
      delete this;
    }
  }

  static void Yield() {
    Coroutine::Suspend();
  }

 private:
  context::Stack stack_;
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
