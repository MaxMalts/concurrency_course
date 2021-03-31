#include <tinyfibers/runtime/scheduler.hpp>

#include <tinyfibers/runtime/stacks.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/panic.hpp>

#include <asio/post.hpp>

#include "timer.hpp"
#include "parking_lot.hpp"

namespace tinyfibers {

//////////////////////////////////////////////////////////////////////

static thread_local Scheduler* current_scheduler;

Scheduler* GetCurrentScheduler() {
  WHEELS_VERIFY(current_scheduler, "Not in fiber context");
  return current_scheduler;
}

struct SchedulerScope {
  SchedulerScope(Scheduler* scheduler) {
    WHEELS_VERIFY(!current_scheduler,
                  "Cannot run scheduler from another scheduler");
    current_scheduler = scheduler;
  }

  ~SchedulerScope() {
    current_scheduler = nullptr;
  }
};

//////////////////////////////////////////////////////////////////////

Scheduler::Scheduler() {
}

Fiber* Scheduler::GetCurrentFiber() {
  WHEELS_VERIFY(running_ != nullptr, "Not in fiber context");
  return running_;
}

void Scheduler::SwitchToScheduler(Fiber* me) {
  me->Context().SwitchTo(loop_context_);
}

// System calls

Fiber* Scheduler::Spawn(FiberRoutine routine) {
  Fiber* newbie = CreateFiber(std::move(routine));
  Schedule(newbie);
  return newbie;
}

void Scheduler::Yield() {
  Fiber* caller = GetCurrentFiber();
  caller->SetState(FiberState::Runnable);
  SwitchToScheduler(caller);
}

void Scheduler::SleepFor(Duration delay) {
  Fiber* caller = GetCurrentFiber();

  WaitableTimer timer(event_loop_, delay);
  timer.async_wait([this, caller](const asio::error_code&) {
    Resume(caller);
  });

  Suspend();
}

void Scheduler::Suspend() {
  Fiber* caller = GetCurrentFiber();
  caller->SetState(FiberState::Suspended);
  SwitchToScheduler(caller);
}

void Scheduler::Resume(Fiber* fiber) {
  WHEELS_ASSERT(fiber->State() == FiberState::Suspended,
                "Expected fiber in Suspended state");
  fiber->SetState(FiberState::Runnable);
  Schedule(fiber);
}

void Scheduler::Terminate() {
  Fiber* caller = GetCurrentFiber();
  caller->SetState(FiberState::Terminated);
  // Leave this context forever
  SwitchToScheduler(caller);
}

// Scheduling

void Scheduler::Run(FiberRoutine init) {
  SchedulerScope scope(this);
  Spawn(std::move(init));
  RunLoop();
}

void Scheduler::RunLoop() {
  event_loop_.run();
}

void Scheduler::Step(Fiber* fiber) {
  running_ = fiber;
  fiber->SetState(FiberState::Running);
  SwitchToFiber(fiber);
  running_ = nullptr;
}

void Scheduler::SwitchToFiber(Fiber* fiber) {
  loop_context_.SwitchTo(fiber->Context());
}

void Scheduler::Reschedule(Fiber* fiber) {
  switch (fiber->State()) {
    case FiberState::Runnable:  // From Yield
      Schedule(fiber);
      break;
    case FiberState::Suspended:  // From Suspend
      // Do nothing
      break;
    case FiberState::Terminated:  // From Terminate
      Destroy(fiber);
      break;
    default:
      WHEELS_PANIC("Unexpected fiber state");
      break;
  }
}

void Scheduler::Schedule(Fiber* fiber) {
  asio::post(event_loop_, [this, fiber]() {
    Step(fiber);
    Reschedule(fiber);
  });
}

Fiber* Scheduler::CreateFiber(FiberRoutine routine) {
  auto stack = AllocateStack();
  FiberId id = ids_.NextId();
  return new Fiber(std::move(routine), std::move(stack), id, event_loop_);
}

void Scheduler::Destroy(Fiber* fiber) {
  ReleaseStack(std::move(fiber->Stack()));
  delete fiber;
}

//////////////////////////////////////////////////////////////////////

Fiber* GetCurrentFiber() {
  return GetCurrentScheduler()->GetCurrentFiber();
}

}  // namespace tinyfibers
