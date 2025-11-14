#define _GNU_SOURCE  // strsignal() (<string.h>)

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

void
segvhandler(int sig)
{
    printf("Handler: caught signal %d (%s).\n", sig, strsignal(sig));  // Unsafe
    fflush(nullptr);  // Unsafe

    _exit(EXIT_FAILURE);
}

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

[[noreturn]] void
bye(char *msg)
{
    if (errno > 0) {
        fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, errno, strerror(errno));
    } else {
        fprintf(stderr, "%s!\n", msg);
    }

    exit(EXIT_FAILURE);
}

int
main(void)
{
    struct sigaction sa = {};
    
    sa.sa_handler = segvhandler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        bye("Sigaction failed!");
    }

    recursive(1);

    return EXIT_SUCCESS;
}
