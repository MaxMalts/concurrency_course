.intel_syntax noprefix

#if (APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

# Make symbols visible for linker
.globl FUNCTION_NAME(Capture)
.globl FUNCTION_NAME(JumpTo)

# Solution starts here
# You need just two instructions: 'movq' and 'ret'

FUNCTION_NAME(Capture):
    mov rax, [rsp]
    mov [rdi], rax
    ret

FUNCTION_NAME(JumpTo):
    mov rax, [rdi]
    mov [rsp], rax
    ret
