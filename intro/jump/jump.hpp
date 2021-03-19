#pragma once

// Local jumps
// 'Local' means that jumps do not cross stack frame boundaries

// Saved execution context
struct JumpContext {
  uint64_t ret_addr;
  uint64_t RBX;
  uint64_t RBP;
  uint64_t RDI;
  uint64_t RSI;
  uint64_t RSP;
  uint64_t R12;
  uint64_t R13;
  uint64_t R14;
  uint64_t R15;
};

// Captures the current execution context into 'ctx'
// 'extern "C"' means "C++ compiler, do not mangle function names"
// https://en.wikipedia.org/wiki/Name_mangling
extern "C" void Capture(JumpContext* ctx);

// Jumps to 'Capture' call that captured provided 'ctx'
// This function does not return
extern "C" void JumpTo(JumpContext* ctx);
