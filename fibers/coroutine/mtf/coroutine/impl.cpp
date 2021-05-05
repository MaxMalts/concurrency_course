#include <mtf/coroutine/impl.hpp>

#include <wheels/support/compiler.hpp>

#include <iostream>

namespace mtf::coroutine::impl {

static thread_local Coroutine* current_coroutine = nullptr;

Coroutine::Coroutine(Routine routine, context::StackView stack)
    : routine_(std::move(routine)) {
  routine_context_.Setup(stack, Trampoline);
}

void Coroutine::Resume() {
  if (completed_) {
    return;
  }

  parent_coroutine_ = current_coroutine;
  current_coroutine = this;
  user_context_.SwitchTo(routine_context_);
  current_coroutine = parent_coroutine_;
  if (exception_) {
    std::rethrow_exception(exception_);
  }
}

void Coroutine::Suspend() {
  if (current_coroutine == nullptr) {
    return;
  }

  current_coroutine->routine_context_.SwitchTo(
      current_coroutine->user_context_);
}

bool Coroutine::IsCompleted() const {
  return completed_;
}

void Coroutine::Trampoline() {
  Coroutine* coroutine = current_coroutine;

  coroutine->routine_context_.AfterStart();

  try {
    coroutine->routine_();
  } catch (...) {
    coroutine->exception_ = std::current_exception();
  }

  coroutine->completed_ = true;
  Suspend();  // Never returns

  WHEELS_UNREACHABLE();
}

}  // namespace mtf::coroutine::impl
