#include <mtf/fibers/stacks.hpp>

#include <atomic>

namespace mtf::fibers {

using context::Stack;

Stack AllocateStack() {
  static const size_t kStackPages = 8;
  return Stack::AllocatePages(kStackPages);
}

void ReleaseStack(Stack stack) {
  Stack released{std::move(stack)};
}

context::Stack StackPool::Acquire() {
  std::lock_guard lock(mutex_);

  if (free_stacks_.empty()) {
    return AllocateStack();
  }

  context::Stack stack = std::move(free_stacks_.back());
  free_stacks_.pop_back();
  return stack;
}

void StackPool::Release(context::Stack stack) {
  std::lock_guard lock(mutex_);
  free_stacks_.push_back(std::move(stack));
}

}  // namespace mtf::fibers
