# AMD64 Linux Kernel Conventions
#
# 1. User-level applications use
# as integer registers for passing the sequence
# %rdi, %rsi, %rdx, %rcx, %r8 and %r9.
#
# The kernel interface uses
# %rdi, %rsi, %rdx, %r10, %r8 and %r9.
#
# 2. A system-call is done via the syscall instruction.
# The kernel destroys registers %rcx and %r11.
#
# 3. The number of the syscall has to be passed in register %rax.
#
# 4. System-calls are limited to six arguments,
# no argument is passed directly on the stack.
#
# 5. Returning from the syscall,
# register %rax contains the result of the system-call.
# A value in the range between -4095 and -1 indicates an error,
# it is -errno.
#
# 6. Only values of class INTEGER or class MEMORY
# are passed to the kernel.
#
# See section A.2
# of System V Application Binary Interface
# AMD64 Architecture Processor Supplement
#
# See x86_64-abi-0.99.pdf.

.data

    str: .ascii "Hello, World!\n"
    strend:

.text

    .global _start
    .type _start, @function

_start:

    mov    $1, %rax    # See unistd_64.h and man 2 write.
    mov    $1, %rdi
    lea    str(%rip), %rsi
    mov    $(strend - str), %rdx    
    syscall

    mov    $60, %rax   # See unistd_64.h and man 2 exit.
    mov    $0, %rdi
    syscall
