/*

NAME

dphsysvsem -- dining philosophers problem experiment with forks and sysv semaphores

SYNOPSIS

Usage: ./dphsysvsem -q 5 -i 0.2 -e 0.1 -t 12.0

-q 5 - quantity of philosophers (default 5)
-i 0.2 - time to think (default 0.2 cpu time seconds)
-e 0.1 - time to eat (default 0.1 cpu time seconds)
-t 12.0 - experiment time (default 12.0 monotonic seconds)

DESCRIPTION

See https://en.wikipedia.org/wiki/Dining_philosophers_problem

*/

#include "gettime.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

// Parameters of dining experiment...

static int QTTY = 5; // Quantity of philosophers
static double TTT = 0.2; // Thinking time in seconds
static double TTE = 0.1; // Eating time in seconds
static double TTL = 12.; // Experiment time in seconds

// System V semaphores id...

static int semid = -1;

// Routines to control timing of dining experiment...

static double sw_monotonic = 0; // Stopwatch for monotonic time

static inline bool checktime() {
    return (gettime_monotonic() - sw_monotonic < TTL);
}

static bool elapsedtime(double *ptr_elapsed_monotonic) {
    *ptr_elapsed_monotonic = gettime_monotonic() - sw_monotonic;
    return (gettime_monotonic() - sw_monotonic < TTL);
}

void business(double duration) {
    double stop = gettime_cpu_process() + duration;
    while (gettime_cpu_process() < stop) {};
}

// Bye on errors...

[[noreturn]] void
bye_usage()
{
    printf("Usage: dph -q 5 -i 0.2 -e 0.1 -t 12.0\n");
    printf("-q 5 - quantity of philosophers (default 5)\n");
    printf("-i 0.2 - time to think (default 0.2 cpu time seconds)\n");
    printf("-e 0.1 - time to eat (default 0.1 cpu time seconds)\n");
    printf("-t 12.0 - experiment time (default 12.0 monotonic seconds)\n");

    exit(EXIT_FAILURE);
}

[[noreturn]] void
bye(char *msg)
{
    if (errno != 0) {
        fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, errno, strerror(errno));
    } else {
        fprintf(stderr, "%s!\n", msg);
    }

    exit(EXIT_FAILURE);
}

// Thinking and eating code for child processes (philosophers)...

void
thinkandeat(int id)
{

    enum {THINK, EAT} state = THINK;

    int fork1 = id % QTTY;  // "fork1" in the "la fourchette" sense
    int fork2 = (id + 1) % QTTY;  // "fork2" in the "la fourchette" sense

    struct sembuf lock[2] = {{.sem_op = -1, .sem_num = fork1}, {.sem_op = -1, .sem_num = fork2},};    
    struct sembuf unlock[2] = {{.sem_op = 1, .sem_num = fork1}, {.sem_op = 1, .sem_num = fork2},};

    while (checktime()) {

        if (state == THINK) {

            // Thinking...
    
            printf("Process %d: thinking...\n", id);
            business(TTT);

            // Going to eat...

            state = EAT;


        } else if (state == EAT) {

            // Locking both forks simultaneously...

            printf("Process %d: trying to lock forks %d, %d...\n", id, fork1, fork2);

            if (semop(semid, lock, 2) == -1) {
                bye("semop failed");
            }

            printf("Process %d: locked forks %d, %d!\n", id, fork1, fork2);

            // Eating...

            printf("Process %d: eating...\n", id);
            business(TTE);

            // Unlocking both forks simultaneously...

            if (semop(semid, unlock, 2) == -1) {
                bye("semop failed");
            }

            printf("Process %d: unlocked forks %d, %d.\n", id, fork1, fork2);

            // Going to think...

            state = THINK;

        }

    }

}

// Creator of the philosophers code...

int
main(int argc, char *argv[])
{

    int opt = 0;

    while ((opt = getopt(argc, argv, ":q:i:e:t:")) != -1) {
        switch (opt) {
        case 'q':
            QTTY = strtol(optarg, nullptr, 10);
            break;
        case 'i':
            TTT = strtod(optarg, nullptr);
            break;
        case 'e':
            TTE = strtod(optarg, nullptr);
            break;                       
        case 't':
            TTL = strtod(optarg, nullptr);
            break;                       
        default:
            bye_usage();
        }
    }

    // Starting stopwatch to measure experiment time...

    sw_monotonic = gettime_monotonic();

    // Creating semaphores...

    semid = semget(IPC_PRIVATE, QTTY, S_IRUSR | S_IWUSR);

    if (semid == -1) {
        bye("semget failed");
    }

    // Initializing semaphores...

    for (int i = 0; i < QTTY; ++i) {
        semctl(semid, i, SETVAL, 1);
    }

    // Creating philosophers...

    for (int child = 0; child < QTTY; ++child) {

        switch (fork()) {
        case -1:
            bye("fork failed");
        case 0:
            thinkandeat(child);
            _exit(EXIT_SUCCESS);
        }

    }

    // Printing experimets time alomst every monotonic second...
    
    double elapsed_monotonic;
    double floored_monotonic = 0;

    while (elapsedtime(&elapsed_monotonic)) {
       if (floor(elapsed_monotonic) > floored_monotonic) {
           floored_monotonic = floor(elapsed_monotonic);
           printf("Dining experiment time %.0f s.\n", elapsed_monotonic);
       }
    };

    // Waiting for children...

    while (wait(nullptr) > 0) {};

    // Removing semaphores...

    if (semctl(semid, 0, IPC_RMID) == -1) {
        bye("semctl IPC_RMID failed");
    };

    // Total experiment time...

    elapsedtime(&elapsed_monotonic);
    printf("Dining experiment total time = %.0f s.\n", elapsed_monotonic);

    return EXIT_SUCCESS;

}
