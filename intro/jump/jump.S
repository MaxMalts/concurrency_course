.intel_syntax noprefix

#if (APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

# Make symbols visible for linker
.globl FUNCTION_NAME(Capture)
.globl FUNCTION_NAME(JumpTo)

FUNCTION_NAME(Capture):
    mov [rdi + 8], rbx
    mov [rdi + 2 * 8], rbp
    mov [rdi + 3 * 8], rdi
    mov [rdi + 4 * 8], rsi
    mov [rdi + 5 * 8], rsp
    mov [rdi + 6 * 8], r12
    mov [rdi + 7 * 8], r13
    mov [rdi + 8 * 8], r14
    mov [rdi + 9 * 8], r15
    mov rax, [rsp]
    mov [rdi], rax
    ret

FUNCTION_NAME(JumpTo):
    mov rax, [rdi]
    mov [rsp], rax
    mov rbx, [rdi + 8]
    mov rbp, [rdi + 2 * 8]
    mov rsi, [rdi + 4 * 8]
    mov rsp, [rdi + 5 * 8]
    mov r12, [rdi + 6 * 8]
    mov r13, [rdi + 7 * 8]
    mov r14, [rdi + 8 * 8]
    mov r15, [rdi + 9 * 8]
    mov rdi, [rdi + 3 * 8]
    ret
