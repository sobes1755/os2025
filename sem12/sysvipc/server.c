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
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/shm.h>

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

    // Creating server's MSG key...

    key_t msgkey = ftok(SERVER_NAME, 1);

    if (msgkey == (key_t) -1) {
        bye("Ftok failed!");
    }

    printf("Server (%d): created MSG key (%#x).\n", getpid(), msgkey);

    // Creating server's MSG queue...

    int msgid = msgget(msgkey, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    if (msgid == -1) {
        bye("Server: msgget failed");
    }

    printf("Server (%d): created MSG queue (%d).\n", getpid(), msgid);

    // Catch some signals....

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

    // Read multipliers and write products...

    while (! terminated) {

        // Reading MSG queue...

        printf("\nServer (%d): receiving client's MSG from server's MSG queue...\n", getpid());

        struct client_message cm = {};

        if (msgrcv(msgid, &cm, client_message_size, 1, 0) != client_message_size) {
            if (terminated)
               break;
            else
               bye("Server: msgrcv failed");
        }

        printf("Server (%d): ...received client's MSG (mtype = %ld, pid = %d, shmid = %d, size1 = %zu, size2 = %zu).\n\n",
            getpid(), cm.mtype, cm.pid, cm.shmid, cm.size1, cm.size2);

        // Attaching client's SHMEM...

        char *shmptr = shmat(cm.shmid, nullptr, 0);

        if (shmptr == (void *) -1) {
            bye("Server: shmat failed");
        }

        printf("Server (%d): attached client's SHMEM (%p).\n", getpid(), shmptr);

        // Reading multipliers...

        char *mul1 = shmptr;
        char *mul2 = shmptr + cm.size1;

        printf("Server (%d): read client's multipliers ", getpid());
        printf("(mul1 = \"%s\", ", mul1);
        printf("mul2 = \"%s\").\n", mul2);

	// Write to python's pipe...

        writeline(out[1], mul1, cm.size1 - 1);
        writeline(out[1], mul2, cm.size2 - 1);

        printf("Server (%d): wrote multipliers to python ", getpid());
        printf("(\"%s\", ", mul1);
        printf("\"%s\").\n", mul2);

        // Read from python's pipe...

        char *product = {};
        size_t product_allocated_size = {};
        ssize_t product_size = {};

        product_size = readline(in[0], &product, &product_allocated_size) + 1;  // + 1 for '\0'

        printf("Server (%d): read product from python ", getpid());
        printf("(\"%s\").\n", product);

        // Write product to client's SHMEM...

        memcpy(shmptr, product, product_size);

        printf("Server (%d): wrote product to client's SHMEM (\"%s\").\n", getpid(), product);

        // Detaching client's SHMEM...

        if (shmdt(shmptr) == -1) {
            bye("Server: shmdt failed");
        }

        printf("Server (%d): detached client's SHMEM (%p).\n", getpid(), shmptr);

        // Writing server's message ro server's MSG queue...

        struct server_message sm = {};

        sm.mtype = cm.pid;
        sm.size = product_size;

        if (msgsnd(msgid, &sm, server_message_size, 0) == -1) {
            bye("Server: msgsnd failed");
        }

        printf("Server (%d): sent MSG (mtype = %ld, size = %zu) to server's MSG queue.\n",
            getpid(), sm.mtype, sm.size);

        free(product);

    }

    // Deleting server's MSG queue...

    if (msgctl(msgid, IPC_RMID, nullptr) == -1) {
        bye("Server: delete MSG queue failed");
    };

    printf("Server (%d): deleted MSG queue (%d).\n\n", getpid(), msgid);

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
