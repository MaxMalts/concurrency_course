#include <tinyfibers/runtime/fiber.hpp>

#include <tinyfibers/runtime/scheduler.hpp>

#include <wheels/support/compiler.hpp>
#include <wheels/support/panic.hpp>
#include <wheels/support/exception.hpp>

#include <asio/io_context.hpp>

namespace tinyfibers {

Fiber::Fiber(FiberRoutine routine, context::Stack&& stack, FiberId id,
             asio::io_context& event_loop)
    : routine_(std::move(routine)),
      stack_(std::move(stack)),
      state_(FiberState::Starting),
      id_(id),
      sleep_timer_(event_loop) {
  SetupTrampoline();
}

Fiber::~Fiber() {
  if (watcher_ != nullptr) {
    watcher_->OnCompleted();
  }
}

void Fiber::Trampoline() {
  // Fiber execution starts here

  Fiber* fiber = GetCurrentFiber();

  // Finalize first context switch
  fiber->Context().AfterStart();

  fiber->SetState(FiberState::Running);

  try {
    fiber->RunUserRoutine();
  } catch (...) {
    WHEELS_PANIC(
        "Uncaught exception in fiber: " << wheels::CurrentExceptionMessage());
  }

  GetCurrentScheduler()->Terminate();  // Never returns

  WHEELS_UNREACHABLE();
}

void Fiber::SetupTrampoline() {
  context_.Setup(
      /*stack=*/stack_.View(),
      /*trampoline=*/Trampoline);
}

}  // namespace tinyfibers
