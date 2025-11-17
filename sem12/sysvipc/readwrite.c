#include "readwrite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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

void
writeline(int fd, char *buffer, size_t size)
{
    size_t pos = 0;

    while (pos < size) {
        ssize_t w = write(fd, &buffer[pos], size - pos);
        if (w == -1)
            bye("Write failed");
        else
           pos += w;
    }

    pos = 0;

    while (pos < 1) {
        ssize_t w = write(fd, "\n", 1);
        if (w == -1)
            bye("Write failed");
        else
           pos += w;
    }
}

ssize_t
readline(int fd, char **pbuffer, size_t *psize)
{
    ssize_t r = getline(pbuffer, psize, fdopen(fd, "r"));

    if (r == -1) {
        bye("Getline error");
    }

    if (r > 0) {
        if ((*pbuffer)[r - 1] == '\n') {
            (*pbuffer)[r - 1] = '\0';
            --r;
        }
    }

    return r;
}
