/*

NAME

pipeline -- creates a pipeline.

SYNOPSIS

pipeline exe1 arg1 exe2 arg2 creates exe1 arg1 | exe2 arg2

DESCRIPTION

Example: pipeline ls -h wc -l.

*/

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

    if (argc != 4 + 1) {
        printf("Usage: exe1 arg1 exe2 arg2\n\n");
        printf("Creates a pipeline: exe1 arg1 | exe2 arg2.\n");
        return EXIT_FAILURE;
    }

    int fd[2];

    if (pipe(fd) == -1) {
        bye("Pipe failed");
    }

    // Creating 1st child...

    switch (fork()) {
    case -1:

        bye("Fork failed");

    case 0:  // 1st child

        // 1st child doesn't read anything from pipe read descriptor (fd[0]),
        // closing pipe read rescriptor (fd[0])...

        if (close(fd[0]) == -1) {
            bye("Child 1: close fd[0] failed"); 
        }

        // Duplicating pipe write descriptor (fd[1]) to STDOUT_FILENO (1) descriptor of 1st child,
        // and closing pipe write descriptor (fd[1])...

        if (fd[1] != STDOUT_FILENO) {
            if (dup2(fd[1], STDOUT_FILENO) == -1) {
                bye("Child 1: dup2 failed");
            }
            if (close(fd[1]) == -1) {
                bye("Child 1: close fd[1] failed");
            }
        }

        execlp(argv[1], argv[1], argv[2], nullptr);

        bye("Child 1: execlp failed");

    }

    // Creating 2nd child...

    switch (fork()) {
    case -1:

        bye("Fork failed");

    case 0:  // 2nd child

        // 2nd child doesn't write anything to pipe write descriptor (fd[1]),
        // closing pipe write rescriptor (fd[1])...

        if (close(fd[1]) == -1) {
            bye("Child 2: close fd[2] failed"); 
        }

        // Duplicating pipe read descriptor (fd[0]) to STDIN_FILENO (0) descriptor of 1st child,
        // and closing pipe read descriptor (fd[0])...

        if (fd[0] != STDIN_FILENO) {
            if (dup2(fd[0], STDIN_FILENO) == -1) {
                bye("Child 2: dup2 failed");
            }
            if (close(fd[0]) == -1) {
                bye("Child 2: close fd[0] failed");
            }
        }

        execlp(argv[3], argv[3], argv[4], nullptr);

        bye("Child 2: execlp failed");

    }

    if (close(fd[0]) == -1) {
        bye("Close fd[0] failed");
    }
    if (close(fd[1]) == -1) {
        bye("Close fd[1] failed");
    }

    if (wait(NULL) == -1) {
        bye("Wait failed");
    }
    if (wait(NULL) == -1) {
        bye("Wait failed");
    }

    return EXIT_SUCCESS;

}
