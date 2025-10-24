/*

NAME

dph -- dining philosophers problem experiment with pthreads and pthread_mutexes

SYNOPSIS

Usage: ./dph -q 5 -i 0.2 -e 0.1 -t 12.0 -y yes -o no -v no

-q 5 - quantity of philosophers (default 5)
-i 0.2 - time to think (default 0.2 cpu time seconds)
-e 0.1 - time to eat (default 0.1 cpu time seconds)
-t 12.0 - experiment time (default 12.0 monotonic seconds)
-y yes|no - use trylock to avoid deadlock (default yes)
-o max|min|no - lock max|min fork first to avoid deadlock (default no)
-v yes|no - print philosopher's status (default no)

DESCRIPTION

dph -q 12 -y yes -- solves the problem with trylocks
dph -q 12 -y no -o max -- solves the problem using locks ordering
dph -q 2 -i 0 -e 0.5 -y no -o no -v yes -- probably catches a deadlock

See https://en.wikipedia.org/wiki/Dining_philosophers_problem

*/

#include "gettime.h"
#include "swap.h"

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int QTTY = 5; // Quantity of philosophers
static double TTT = 0.2; // Thinking time in seconds
static double TTE = 0.1; // Eating time in seconds
static double TTL = 12.; // Experiment time in seconds
static bool V = false; // Verbose
static int ORDER = 0;
static bool TRYLOCK = true;

typedef enum {THINK, EAT} state;

static pthread_t *thread; // Threads for philosophers
static pthread_mutex_t *mutex; // Mutexes for forks (philosopher needs two forks to eat)

static double *think; // Time spent for thinking
static double *eat; // Time spent for eating
static double *live; // Lifetime (monotonic)
static double *cpu; // Lifetime (cpu time)

// Control time of experiment

static double sw_cpu_process; // Stopwatch for process cpu time
static double sw_monotonic; // Stopwatch for monotonic time

static inline bool thread_checktime() {
    return (gettime_monotonic() - sw_monotonic < TTL);
}

static bool process_checktime(double *ptr_elapsed_monotonic, double *ptr_elapsed_cpu_process) {
    *ptr_elapsed_monotonic = gettime_monotonic() - sw_monotonic;
    *ptr_elapsed_cpu_process = gettime_cpu_process() - sw_cpu_process;
    return (gettime_monotonic() - sw_monotonic < TTL);
}

// Spend some cpu time (duration in seconds)

void business(double duration) {
    double stop = gettime_cpu_thread() + duration;
    while (gettime_cpu_thread() < stop) {};
}

// Thread function WITHOUT pthread_trylock

void *
thread_start(void *ptr)
{
    int id = (intptr_t) ptr;

    live[id] -= gettime_monotonic();
    cpu[id] -= gettime_cpu_thread();

    int fork1 = id % QTTY;
    int fork2 = (id + 1) % QTTY;

    if (ORDER > 0) {
        if (fork1 < fork2) SWAP(fork1, fork2);
    } else if (ORDER < 0) {
        if (fork1 > fork2) SWAP(fork1, fork2);
    }

    while (thread_checktime()) {

        // Thinking...
    
        think[id] -= gettime_cpu_thread();
        if (V) printf("Thread %d: thinking!\n", id);
        business(TTT);
        think[id] += gettime_cpu_thread();

        // Locking forks...

        if (pthread_mutex_lock(mutex + fork1) != 0) {
            perror("Pthread mutex lock failed");
            exit(EXIT_FAILURE);
        }
        if (V) printf("Thread %d: locked fork %d!\n", id, fork1);

        if (pthread_mutex_lock(mutex + fork2) != 0) {
            perror("Pthread mutex lock failed");
            exit(EXIT_FAILURE);
        }
        if (V) printf("Thread %d: locked fork %d!\n", id, fork2);

    // Eating...
    
        eat[id] -= gettime_cpu_thread();
        if (V) printf("Thread %d: eating!\n", id);
        business(TTE);
        eat[id] += gettime_cpu_thread();

        // Unlocking forks...

        if (pthread_mutex_unlock(mutex + fork2) != 0) {
            perror("Pthread mutex unlock lock failed");
            exit(EXIT_FAILURE);
        }
        if (V) printf("Thread %d: unlocked fork %d!\n", id, fork2);

        if (pthread_mutex_unlock(mutex + fork1) != 0) {
            perror("Pthread mutex unlock lock failed");
            exit(EXIT_FAILURE);
        }
        if (V) printf("Thread %d: unlocked fork %d!\n", id, fork1);

    }

    cpu[id] += gettime_cpu_thread();
    live[id] += gettime_monotonic();

    return nullptr;
}

// Thread function WITH pthread_trylock

void *
thread_start_trylock(void *ptr)
{

    int id = (intptr_t) ptr;

    live[id] -= gettime_monotonic();
    cpu[id] -= gettime_cpu_thread();

    int fork1 = id % QTTY;
    int fork2 = (id + 1) % QTTY;

    if (ORDER > 0) {
        if (fork1 < fork2) SWAP(fork1, fork2);
    } else if (ORDER < 0) {
        if (fork1 > fork2) SWAP(fork1, fork2);
    }

    state s = THINK;

    while (thread_checktime()) {

        if (s == THINK) {

            // Thinking...
    
            think[id] -= gettime_monotonic();
            if (V) printf("Thread %d: thinking!\n", id);
            business(TTT);
            s = EAT;
            think[id] += gettime_monotonic();


        } else if (s == EAT) {

            // Locking 1st fork...

            if (pthread_mutex_lock(mutex + fork1) != 0) {
                perror("Pthread mutex lock failed");
                exit(EXIT_FAILURE);
            }
            if (V) printf("Thread %d: locked fork %d!\n", id, fork1);

            // Trying to lock 2nd fork...

            int trylock = pthread_mutex_trylock(mutex + fork2);

            if (trylock == 0) {

                if (V) printf("Thread %d: locked fork %d!\n", id, fork2);

                // Eating...

                eat[id] -= gettime_monotonic();
                if (V) printf("Thread %d: eating!\n", id);
                business(TTE);
                s = THINK;
                eat[id] += gettime_monotonic();

                // Unlocking 2nd fork...

                if (pthread_mutex_unlock(mutex + fork2) != 0) {
                    perror("Pthread mutex unlock lock failed");
                    exit(EXIT_FAILURE);
                }
                if (V) printf("Thread %d: unlocked fork %d!\n", id, fork2);

            } else if (trylock == EBUSY) {

                // 2nd fork is busy...        

            } else {

                // Trylock error...

                perror("Pthread mutex try lock failed");
                exit(EXIT_FAILURE);

            }

            // Unlocking 1st fork...

            if (pthread_mutex_unlock(mutex + fork1) != 0) {
                perror("Pthread mutex unlock lock failed");
                exit(EXIT_FAILURE);
            }
            if (V) printf("Thread %d: unlocked fork %d!\n", id, fork1);

        }

    }

    cpu[id] += gettime_cpu_thread();
    live[id] += gettime_monotonic();

    return nullptr;
}

// Main thread and friends...

[[noreturn]] void
exit_on_badusage()
{
    printf("Usage: dph -q 5 -i 0.2 -e 0.1 -t 12.0 -y yes -o no -v no\n\n");
    printf("-q 5 - quantity of philosophers (default 5)\n");
    printf("-i 0.2 - time to think (default 0.2 cpu time seconds)\n");
    printf("-e 0.1 - time to eat (default 0.1 cpu time seconds)\n");
    printf("-t 12.0 - experiment time (default 12.0 monotonic seconds)\n");
    printf("-y yes|no - use trylock to avoid deadlock (default yes)\n");
    printf("-o max|min|no - lock max|min fork first to avoid deadlock (default no)\n");
    printf("-v yes|no - print philosopher's status (default no)\n");

    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{

    int opt = 0;

    while ((opt = getopt(argc, argv, ":q:i:e:t:v:o:y:")) != -1) {
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
        case 'y':
            if (strcmp(optarg, "yes") == 0) {
                TRYLOCK = true;
                break;
            } else if (strcmp(optarg, "no") == 0) {
                TRYLOCK = false;
                break;
            } else {
                exit_on_badusage();
            }
        case 'o':
            if (strcmp(optarg, "max") == 0) {
                ORDER = 1;
                break;
            } else if (strcmp(optarg, "min") == 0) {
                ORDER = -1;
                break;
            } else if (strcmp(optarg, "no") == 0) {
                ORDER = 0;
                break;
            } else {
                exit_on_badusage();
            }
        case 'v':
            if (strcmp(optarg, "yes") == 0) {
                V = true;
                break;
            } else if (strcmp(optarg, "no") == 0) {
                V = false;
                break;
            } else {
                exit_on_badusage();
            }
        default:
            exit_on_badusage();
        }
    }

    // Starting stopwatch for measuring process time...

    sw_monotonic = gettime_monotonic();
    sw_cpu_process = gettime_cpu_process();

    // Allocating heap...

    if (! (thread = calloc(QTTY, sizeof(*thread)))) {
        perror("Calloc for threads failed");
        return EXIT_FAILURE;
    }
    if (! (mutex = calloc(QTTY, sizeof(*mutex)))) {
        perror("Calloc for mutexes failed");
        return EXIT_FAILURE;
    }

    if (! (live = calloc(QTTY, sizeof(*live)))) {
        perror("Calloc for live failed");
        return EXIT_FAILURE;
    }
    if (! (think = calloc(QTTY, sizeof(*think)))) {
        perror("Calloc for think failed");
        return EXIT_FAILURE;
    }
    if (! (eat = calloc(QTTY, sizeof(*eat)))) {
        perror("Calloc for eat failed");
        return EXIT_FAILURE;
    }
    if (! (cpu = calloc(QTTY, sizeof(*cpu)))) {
        perror("Calloc for cpu failed");
        return EXIT_FAILURE;
    }

    // Initializing mutexes...

    for (int i = 0; i < QTTY; ++i) {
        pthread_mutex_init(mutex + i, nullptr);    // Always successful
    }

    // Creating threads...

    if (TRYLOCK) {
        for (int i = 0; i < QTTY; ++i) {
            if (pthread_create(thread + i, nullptr, thread_start_trylock, (void *) (intptr_t) i) != 0) {
                perror("Failed to create thread");
                return EXIT_FAILURE;
            }    
        }
    } else {
        for (int i = 0; i < QTTY; ++i) {
            if (pthread_create(thread + i, nullptr, thread_start, (void *) (intptr_t) i) != 0) {
                perror("Failed to create thread");
                return EXIT_FAILURE;
            }
        }
    }

    // Printing cpu usage statistics every monotonic second...
    
    double elapsed_monotonic;
    double elapsed_cpu_process;

    double f = 0;

    while (process_checktime(&elapsed_monotonic, &elapsed_cpu_process)) {
       if (floor(elapsed_monotonic) > f) {
           f = floor(elapsed_monotonic);
           printf("Process: time %.0f s, cpu time %f s.\n", elapsed_monotonic, elapsed_cpu_process);
       }
    };

    // Waiting for threads...

    for (int i = 0; i < QTTY; ++i) {
        pthread_join(*(thread + i), nullptr);
    }

    for (int i = 0; i < QTTY; ++i) {
        pthread_mutex_destroy(mutex + i);    // Does almost nothing in Linux
    }

    // Printing threads (except main threads) timing...

    for (int i = 0; i < QTTY; ++i) {
        printf("Thread %d: cpu time %f s, time %f s (thought %.2f %%, ate %.2f %%, other %.2f %%).\n",
            i + 1, cpu[i], live[i],
            think[i] / live[i] * 100,
            eat[i] / live[i] * 100,
            (1. - (think[i] + eat[i]) / live[i]) * 100);
    }

    // Calculating and printing total threads (except main thread) timing...

    for (int i = 1; i < QTTY; ++i) {
        live[0] += live[i];
        cpu[0] += cpu[i];
        think[0] += think[i];
        eat[0] += eat[i];
    }

    printf("Threads: cpu time %f s, time %f s (thought %.2f %%, ate %.2f %%, other %.2f %%).\n",
            cpu[0], live[0],
            think[0] / live[0] * 100,
            eat[0] / live[0] * 100,
            (1. - (think[0] + eat[0]) / live[0]) * 100);
    
    // Printing process timing...

    printf("Process: cpu time %f s, time %f s.\n",
    gettime_cpu_process() - sw_cpu_process,
    gettime_monotonic() - sw_monotonic);

    free(cpu);
    free(eat);
    free(think);
    free(live);

    free(mutex);
    free(thread);

    return EXIT_SUCCESS;

}
