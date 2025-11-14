#include <stdio.h>
#include <stdlib.h>

[[noreturn]] void
recursive(int level)
{
    [[maybe_unused]] char data_on_stack[65536];

    void *rbp;
    void *rsp;

    __asm__ volatile (
        "mov %%rbp, %0"  // Assembly instructions
        : "=r" (rbp)     // Output operands
        :                // Input operands
        :                // Clobbered registers
    );
    __asm__ volatile (
        "mov %%rsp, %0"
        : "=r" (rsp)
        :
        :
    );

    printf("Recursive: level = %d, rbp = %p, rsp = %p.\n", level, rbp, rsp);

    recursive(level + 1);
}

int
main(void)
{
    recursive(1);
    return EXIT_SUCCESS;
}
