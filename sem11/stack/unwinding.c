#define _GNU_SOURCE  // strsignal() (<string.h>)

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

static sigjmp_buf env;

void
segvhandler(int sig)
{
    printf("Handler caught signal %d: \"%s\".\n", sig, strsignal(sig));  // Unsafe
    fflush(nullptr);  // Unsafe

    siglongjmp(env, 1);

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
    stack_t stack = {};

    stack.ss_size = SIGSTKSZ;
    stack.ss_flags = 0;
    stack.ss_sp = malloc(stack.ss_size);

    if (stack.ss_sp == nullptr) {
        bye("Malloc failed!");
    }
    if (sigaltstack(&stack, NULL) == -1) {
        bye("Sigaltstack failed!");
    }

    struct sigaction sa = {};
   
    sa.sa_handler = segvhandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        bye("Sigaction failed!");
    }

    sigsetjmp(env, 1);

    int choice = {};

    printf("Enter 1 to overflow stack.\n");
    printf("Enter 0 to exit\n");

    scanf("%d", &choice);

    switch (choice) {
    case 1:
        recursive(1);
    default:
        break;
    }

    return EXIT_SUCCESS;
}
