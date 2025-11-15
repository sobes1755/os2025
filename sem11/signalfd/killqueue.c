#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
main(int argc, char *argv[])
{
    if (argc < 3) {
        bye("Usage: killqueue pid sig [value]");
    }

    int pid = strtol(argv[1], nullptr, 10);
    int sig = strtol(argv[2], nullptr, 10);
    int value = (argc > 3)? strtol(argv[3], nullptr, 10): 0;

    union sigval sv = {.sival_int = value};

    int r = sigqueue(pid, sig, sv);

    if (r == -1) {
        bye("Sigqueue failed");
    }

    printf("Process %d sent signal:\n\tpid = %d, sig = %d, sival_int = %d.\n",
        getpid(), pid, sig, value);

    return EXIT_SUCCESS;
}
