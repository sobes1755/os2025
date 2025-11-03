/*

NAME

pipe_ab_size -- experiment with pipe buffer size (is write atomic or not?)

SYNOPSIS

pipe_ab_size num_proc buf_mult

DESCRIPTION

Creates a pipe and num_proc child processes,
each child process writes 'A...A\n', ..., 'Z...Z\n' to pipe
in bunches of buf_mult * PIPE_BUF bytes,
parent process reads from the pipe and writes to stdout.

*/

#define _POSIX_C_SOURCE 200809L  // PIPE_BUF

#include <errno.h>
#include <limits.h>  // PIPE_BUF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

[[noreturn]] void
bye(char *msg)
{
    fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    if (argc != 2 + 1) {

        printf("Usage: pipe_ab_size num_proc buf_mult\n\n");
        printf("Creates a pipe and num_proc child processes,\n");
        printf("each child process writes 'A...A\\n', ..., 'Z...Z\\n' to pipe\n");
        printf("in bunches of buf_mult * PIPE_BUF bytes,\n");
        printf("parent process reads from the pipe and writes to stdout.\n");

        return EXIT_FAILURE;

    }

    int num_proc = strtol(argv[1], nullptr, 10);
    double buf_mult = strtod(argv[2], nullptr);

    // Creating pipe...

    int fd[2];

    if (pipe(fd) == -1) {
        bye("Pipe failed");
    }

    // Creating childs...

    for (int child = 0; child < num_proc; ++child) {

        switch (fork()) {
        case -1:

            bye("Fork failed");

        case 0:

            // Child writes to pipe...

            if (close(fd[0]) == -1) {
                bye("Child: close fd[0] failed");
            }

            ssize_t write_buffer_size = PIPE_BUF * buf_mult;
            char *write_buffer = calloc(write_buffer_size, sizeof(*write_buffer));

            for (char c = 'A'; c <= 'Z'; ++c) {

                memset(write_buffer, c, write_buffer_size);
                write_buffer[write_buffer_size - 1] = '\n';

                if (write(fd[1], write_buffer, write_buffer_size) != write_buffer_size) {
                    bye("Child: write fd[1] failed");
                }

            }

            free(write_buffer);

            if (close(fd[1]) == -1) {
                bye("Child: close fd[1] failed");
            }

            _exit(EXIT_SUCCESS);

        }
    }

    // Parent reads from pipe...

    if (close(fd[1]) == -1) {
        bye("Close fd[1] failed");
    }
    
    ssize_t read_buffer_size = 65536;
    char *read_buffer = calloc(read_buffer_size, sizeof(*read_buffer));

    for (;;) {

        ssize_t r = read(fd[0], read_buffer, read_buffer_size);

        if (r == 0) {

            break;

        } else if (r > 0) {

            ssize_t w = write(STDOUT_FILENO, read_buffer, r);

            if (w != r)  {
                bye("Write failed");
            }

        } else if (r < 0) {

            bye("Read failed");

        }

    }

    free(read_buffer);

    if (close(fd[0]) == -1) {
        bye("Close fd[0] failed");
    }

    exit(EXIT_SUCCESS);

}
