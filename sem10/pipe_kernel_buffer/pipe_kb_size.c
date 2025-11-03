/*

NAME

pipe_kb_size -- pipe kernel buffer size experiment

SYNOPSIS

pipe_kb_size

DESCRIPTION

Creates pipe, writes incrementing portions of bytes to pipe,
reads them: in a while write goes to wait state.

*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ONE_MIB 1048576
char buffer[ONE_MIB] = {};

[[noreturn]] void
bye(char *msg)
{
    fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

int
main(void)
{
    int fd[2];

    if (pipe(fd) == -1) {
        bye("Pipe failed");
    }

    for (ssize_t size = 0; size <= ONE_MIB; ++size) {

        printf("Writing %zd bytes to pipe... ", size); fflush(stdout);
        ssize_t w = write(fd[1], buffer, size);
        printf("Wrote %zd bytes to pipe.\n", w);

        if (w != size) {
            bye("Write failed");
        }

        printf("Reading %zd bytes from pipe... ", size); fflush(stdout);
        ssize_t r = read(fd[0], buffer, size);
        printf("Read %zd bytes from pipe.\n", w);

        if (r != size) {
            bye("Read failed");
        }

    }

    if (close(fd[1]) == -1) {
        bye("Close fd[1] failed");
    }
    if (close(fd[0]) == -1) {
        bye("Close fd[0] failed");
    }

    exit(EXIT_SUCCESS);
}
