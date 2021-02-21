.intel_syntax noprefix

#if(APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicLoad)
.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)

FUNCTION_NAME(AtomicLoad):
    mov rax, [rdi]
    ret

FUNCTION_NAME(AtomicStore):
    mov [rdi], rsi
    ret

FUNCTION_NAME(AtomicExchange):
    xchg [rdi], rsi  # Store
    mov rax, rsi  # Load
    ret
