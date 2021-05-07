#pragma once

#include <context/stack.hpp>

#include <mtf/fibers/spinlock.hpp>
#include <mutex>

#include <vector>

namespace mtf::fibers {

context::Stack AllocateStack();
void ReleaseStack(context::Stack stack);

class StackPool {
 public:
  context::Stack Acquire();

  void Release(context::Stack stack);

 private:
  std::vector<context::Stack> free_stacks_;
  TASSpinLock mutex_;
};

}  // namespace mtf::fibers
