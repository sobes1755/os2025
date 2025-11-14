/*

NAME

malloc -- shows unsafeness of malloc family functions in signal handler

SYNOPSIS

malloc

Catches SIGUSR1, SIGUSR2, SIGTERM.

SIGUSR1 is handled without mallocs.
SIGUSR2 is handled with malloc (and causes errors sometimes).
SIGTERM caught three times will terminate the program.

DESCRIPTION

Example:

./malloc &
PID=$!
kill -s SIGUSR1 $PID
kill -s SIGUSR2 $PID
kill -s SIGTERM $PID
kill -s SIGTERM $PID
kill -s SIGTERM $PID

*/

#define _POSIX_C_SOURCE 199309L  // 

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

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

volatile sig_atomic_t SIGUSR1s = {};
volatile sig_atomic_t SIGUSR2s = {};
volatile sig_atomic_t SIGTERMs = {};

static void
handler(int sig)
{
    if (sig == SIGUSR1) {
        if (SIGUSR1s < SIG_ATOMIC_MAX)
            ++SIGUSR1s;
    } else if (sig == SIGUSR2) {
        char *s = calloc(65536, sizeof(*s));
        if (SIGUSR2s < SIG_ATOMIC_MAX)
            ++SIGUSR2s;
        free(s);
    } else if (sig == SIGTERM) {
        if (SIGTERMs < SIG_ATOMIC_MAX)
            ++SIGTERMs;
    }
}

int
main(void)
{
    struct sigaction sa;

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, nullptr) == -1) {
        bye("Sigaction failed at SIGUSR1");
    }
    if (sigaction(SIGUSR2, &sa, nullptr) == -1) {
        bye("Sigaction failed at SIGUSR2!");
    }
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        bye("Sigaction failed at SITERM!");
    }

    char *a[256];

    for (int i = 0; i < 256; ++i) {
        a[i] = nullptr;
    }

    unsigned int good_mallocs = 0;
    unsigned int bad_mallocs = 0;

    while (SIGTERMs < 3) {

        int i = rand() % 256;

        free(a[i]);

        a[i] = calloc(rand() % 65535 + 1, sizeof(*a[i]));

        if (a[i] == nullptr)
            bad_mallocs ++;
        else
            good_mallocs ++;

        printf("Ptr = %p, good_callocs = %u, bad_callocs = %u, SIGUSR1s = %d, SIGUSR2s = %d, SIGTERMs = %d.\n",
            a[i], good_mallocs, bad_mallocs, SIGUSR1s, SIGUSR2s, SIGTERMs);

    };

    for (int i = 0; i < 256; ++i) {
        free(a[i]);
    }

    return EXIT_SUCCESS;

}
