#include "readwrite.h"

#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

volatile sig_atomic_t terminated = 0;

static void
handler([[maybe_unused]] int sig)
{
    if (terminated < SIG_ATOMIC_MAX)
        ++terminated;
}

int
main(void)
{
    // Creating pipes to communicate with python...

    int out[2];  // Pipe to write to python
    int in[2];   // Pipe to read from python

    if (pipe(out) == -1) {
        bye("Pipe (out) failed");
    }
    if (pipe(in) == -1) {
        bye("Pipe (in) failed");
    }

    printf("Server (%d): created python's pipes.\n", getpid());

    // Fork&exec for python process...

    int py_pid = fork();

    switch (py_pid) {
    case -1:

        bye("Fork failed");

    case 0:

        printf("Server (%d): child (%d) is forked for python's process.\n", getppid(), getpid());

        const int *const py_in = out;
        const int *const py_out = in;

        if (close(py_in[1]) == -1) {
            bye("Python: close py_in[1] failed"); 
        }
        if (close(py_out[0]) == -1) {
            bye("Python: close py_out[0] failed"); 
        }

        if (py_in[0] != STDIN_FILENO) {
            if (dup2(py_in[0], STDIN_FILENO) == -1) {
                bye("Python: dup2 py_in[0] failed");
            }
            if (close(out[0]) == -1) {
                bye("Python: close out[0] failed");
            }
        }
        if (py_out[1] != STDOUT_FILENO) {
            if (dup2(py_out[1], STDOUT_FILENO) == -1) {
                bye("Python: dup2 py_out[1] failed");
            }
            if (close(py_out[1]) == -1) {
                bye("Python: close py_out[1] failed");
            }
        }

        execlp("./abacus.py", "./abacus.py", nullptr);

        bye("Python: execlp failed");

    }

    if (close(out[0]) == -1) {
        bye("Close out[0] failed");
    }
    if (close(in[1]) == -1) {
        bye("Close in[1] failed");
    }

    // Testing python on trivial example...

    char pi[] = "31415926535897932384626433832795028841971693993751058209749445923"
                 "0781640628620899862803482534211706798214808651328230664793844609"
                 "5505822317253594812848111745028410270193852110555964462294895493"
                 "0381964428810975665933446128475648233786783165271201914564856692";
    char e[] = "27182818284590452353602874713526624977572470936999595749669676277"
                "2407663035354759457138217852516642742746639193200305992181741359"
                "6629043572900334295260595630738132328627943490763233829880753195"
                "2510190115738341879307021540891499348841675092447614606680822648";

    char pie[] = "85397342226735670654635508695465744950348885357651149618796011301"
                  "7922861115733080757256386971047394391377494251167746764861705246"
                  "2257660624392903253564545174992164009254235907798252969925155321"
                  "4195728121559087405484666693100917606964143184867605784193380520"
                  "5696599962057617190694788971572872169374503200386755766749617926"
                  "1470118143074734909269963655897743601863892308905646828185793993"
                  "9837209367071366083902130630975593628518603682639994875392549592"
                  "5293189131950660828196963069596931422322070979252758852787960416";

    printf("Server (%d): testing python...\n", getpid());

    writeline(out[1], pi, sizeof(pi) - 1);
    printf("\nServer (%d): wrote pi to python's pipe (\"%s\").\n", getpid(), pi);
    writeline(out[1], e, sizeof(e) - 1);
    printf("Server (%d): wrote e to python's pipe (\"%s\").\n\n", getpid(), e);

    char *buffer = {};
    size_t buffer_size = {};

    readline(in[0], &buffer, &buffer_size);
    printf("\nServer (%d): read pi * e from python's pipe (\"%s\").\n\n", getpid(), buffer);

    if (strcmp(buffer, pie) != 0) {
        bye("Multiplication test failed!");
    }

    printf("Server (%d): ...python tested.\n\n", getpid());

    free(buffer);

    // Creating server's FIFO...

    char *sfifo = SERVER_FIFO;

    mode_t umask_save = umask(0);
    if ((mkfifo(sfifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1) && (errno != EEXIST)) {
        bye("Server: mkfifo failed");
    }
    umask(umask_save);

    printf("Server (%d): created FIFO (\"%s\").\n", getpid(), sfifo);

    // Openning server FIFO...

    int sfd[2];
    
    sfd[0] = open(SERVER_FIFO, O_RDONLY | O_NONBLOCK);
    if (sfd[0] == -1) {
        bye("Server: open O_RDONLY failed");
    }

    int flags = fcntl(sfd[0], F_GETFL, 0);
    if (flags == -1) {
        bye("Server: fcntl F_GETFL failed");
    }
    flags &= ~O_NONBLOCK;
    if (fcntl(sfd[0], F_SETFL, flags) == -1) {
        bye("Server: fcntl F_SETFL failed");
    }

    printf("Server (%d): opened FIFO (%d) for reading.\n", getpid(), sfd[0]);

    sfd[1] = open(SERVER_FIFO, O_WRONLY);
    if (sfd[1] == -1) {
        bye("Server: open O_WRONLY failed");
    }

    printf("Server (%d): opened FIFO (%d) for (dummy) writing.\n", getpid(), sfd[1]);

    // Ignoring write to pipe errors...

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        bye("Server: signal(SIGPIPE, SIG_IGN) failed");
    }

    // Read multipliers and write products...

    struct sigaction sa = {};
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        bye("Sigaction (SIGINT) failed");
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        bye("Sigaction (SIGINT) failed");
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        bye("Sigaction (SIGINT) failed");
    }

    //

    while (! terminated) {

        // Reading header...

        printf("\nServer (%d): reading client header on server's FIFO...\n", getpid());

        struct client_header ch = {};

        if (read(sfd[0], &ch, sizeof(ch)) != sizeof(ch)) {
            if (terminated)
               break;
            else
               bye("Server: client header read failed");
        }

        printf("Server (%d): ...read client's header (pid = %d, size1 = %zu, size2 = %zu).\n",
            getpid(), ch.pid, ch.size1, ch.size2);

        // Reading multipliers...

        printf("\nServer (%d): reading client's (%d) multipliers on server's FIFO...\n",
            getpid(), ch.pid);

        char *mul1 = calloc(ch.size1, sizeof(*mul1));
        char *mul2 = calloc(ch.size2, sizeof(*mul2));

        if (read(sfd[0], mul1, ch.size1) != (ssize_t) ch.size1) {
            if (terminated)
                break;
            else
                bye("Server: mul1 read failed");
        }
        if (read(sfd[0], mul2, ch.size2) != (ssize_t) ch.size2) {
            if (terminated)
                break;
            else
                bye("Server: mul2 read failed");
        }

        printf("Server (%d): ...read client's (%d) multipliers ", getpid(), ch.pid);
        printf("(mul1 = \"%s\", ", mul1);
        printf("mul2 = \"%s\").\n", mul2);

	// Write to python's pipe...

        writeline(out[1], mul1, ch.size1);
        writeline(out[1], mul2, ch.size2);

        printf("\nServer (%d): wrote multipliers to python ", getpid());
        printf("(\"%s\", ", mul1);
        printf("\"%s\").\n", mul2);

        free(mul2);
        free(mul1);

        // Read from python's pipe...

        char *product = {};
        size_t product_allocated_size = {};
        ssize_t product_size = {};

        product_size = readline(in[0], &product, &product_allocated_size);

        printf("Server (%d): read product from python ", getpid());
        printf("(\"%s\").\n\n", product);

        // Open client's FIFO...

        char cfifo[CLIENT_FIFO_LENGTH];
        snprintf(cfifo, sizeof(cfifo), CLIENT_FIFO, ch.pid);

        int cfd1 = open(cfifo, O_WRONLY | O_NONBLOCK);
        if (cfd1 == -1) {
            printf("Server (%d): client's FIFO (\"%s\") open failed", getpid(), cfifo);
            continue;
        }

        printf("Server (%d): opened client's FIFO (\"%s\").\n", getpid(), cfifo);

        // Write server's header to client's FIFO...

        struct server_header sh = {};
        sh.size = product_size;

        if (write(cfd1, &sh, sizeof(sh)) != sizeof(sh)) {
            printf("Server (%d): server's header write to client's FIFO failed", getpid());
        };

        printf("Server (%d): wrote server's header to client's FIFO (size = %zu).\n", getpid(), sh.size);

        // Write product to client's FIFO...

        if (write(cfd1, product, product_size) != (ssize_t) product_size) {
            printf("Server (%d): product write to client's FIFO failed", getpid());
        }

        printf("Server (%d): wrote product to client's FIFO (\"%s\").\n", getpid(), product);

        free(product);

        // Close client FIFO...

        if (close(cfd1) == -1) {
            printf("Server (%d): client's FIFO (\"%s\") close failed", getpid(), cfifo);
            continue;
        }

        printf("Server (%d): closed client's FIFO (\"%s\").\n", getpid(), cfifo);
 
    }

    // Closing server's FIFO...

    if (close(sfd[1]) == -1) {
        bye("Server: FIFO close (O_WRDONLY) failed");
    }

    printf("\nServer (%d): closed (dummy) FIFO (%d).\n", getpid(), sfd[1]);

    if (close(sfd[0]) == -1) {
        bye("Server: FIFO close (O_RDONLY) failed");
    }

    printf("Server (%d): closed FIFO (%d).\n", getpid(), sfd[0]);

    // Deleting server's FIFO...

    if (unlink(sfifo) == -1) {
        bye("Server: server's FIFO unlink failed");
    };

    printf("Server (%d): deleted server's FIFO (\"%s\").\n\n", getpid(), sfifo);

    // Terminating python...

    kill(py_pid, SIGTERM);
    waitpid(py_pid, nullptr, 0);

    printf("Server (%d): terminated python process (%d).\n", getpid(), py_pid);

    // Closing python's pipes...

    if (close(out[1]) == -1) {
        bye("Close out[1] failed");
    }
    if (close(in[0]) == -1) {
        bye("Close in[0] failed");
    }

    printf("Server (%d): closed python's pipes.\n", getpid());

    return EXIT_SUCCESS;

}
