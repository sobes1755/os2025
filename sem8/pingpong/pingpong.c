/*

NAME

pingpong -- pthreads one by one increase value of common variable using pthread_condvar

SYNOPSIS

Usage: pingpong -q 5 -d 0.25 -a 0 -z 64

-q 5 -- quantity of threads (default 5)
-d 0.25 -- time to eat (default 0.25 monotonic seconds)
-a 0 -- value to start with (default 0)
-z 64 -- value to finish with (default 64)

DESCRIPTION

pingpong -q 2 -d 0.25 -a 0 -z 10 -- resembles playing pingpong

*/

#define _POSIX_C_SOURCE 199309L // nanosleep

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

static int qtty = 5;
static struct timespec delay = {.tv_sec = 0, .tv_nsec = 0.25 * 1e9};
static int a = 0;
static int z = 64;

static pthread_t *thread;
static pthread_cond_t *cond;
static pthread_mutex_t *mutex;
static bool *play;

static int hit = 0;

// Thread function

void
*thread_start(void *ptr)
{
    int id = (intptr_t) ptr;
    int next = (id + 1) % qtty;

    while (hit < z) {

        // Waiting for our time to play...

        if (pthread_mutex_lock(mutex + id) != 0) {
            perror("Pthread_mutex_lock failed");
            exit(EXIT_FAILURE);
        }
        while (! play[id]) {
            if (pthread_cond_wait(cond + id, mutex + id) != 0) {
                perror("Pthread_cond_wait failed");
                exit(EXIT_FAILURE);
            }
        }

        // Playing...

        if (hit < z) {

            printf("Thread %d: input %d, ", id + 1, hit);
            ++hit;
            nanosleep(&delay, nullptr);
            printf("output %d.\n", hit);

        }

        // Finishing playing...

        play[id] = false;
        if (pthread_mutex_unlock(mutex + id) != 0) {
            perror("Pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }

        // Waking up next player...    

        if (pthread_mutex_lock(mutex + next) != 0) {
            perror("Pthread_mutex_lock failed");
            exit(EXIT_FAILURE);
        }

        play[next] = true;
        if (pthread_cond_signal(cond + next) != 0) {
            perror("Pthread_cond_signal failed");
            exit(EXIT_FAILURE);
        }
        if (pthread_mutex_unlock(mutex + next) != 0) {
            perror("Pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }

    }

    return nullptr;
}

// Main thread and friends...

[[noreturn]] void
exit_on_badusage()
{
    char *usage =

"Usage: pingpong -q 5 -d 0.25 -a 0 -z 6\n\n"
"-q 5 -- quantity of philosophers (default 5)\n"
"-d 0.25 -- time to eat (default 0.1 cpu time seconds)"
"-a 0 -- value to start with (default 0)"
"-z 64 -- value to finish with (default 64)";

    printf("%s", usage);

    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int opt = 0;

    while ((opt = getopt(argc, argv, ":q:d:a:z:")) != -1) {
        switch (opt) {
        case 'q':
            qtty = strtol(optarg, nullptr, 10);
            break;
        case 'd':
            double sint, sfrac;
            sfrac = modf(strtod(optarg, nullptr), &sint);
            delay.tv_sec = sint;
            delay.tv_nsec = sfrac * 1e9;
            break;
        case 'a':
            a = strtol(optarg, nullptr, 10);
            break;
        case 'z':
            z = strtol(optarg, nullptr, 10);
            break;
        default:
            exit_on_badusage();
        }
    }

    // Allocating some heap...

    if (! (thread = calloc(qtty, sizeof(*thread)))) {
        perror("Calloc for threead failed");
        return EXIT_FAILURE;
    };
    if (! (cond = calloc(qtty, sizeof(*cond)))) {
        perror("Calloc for threead failed");
        return EXIT_FAILURE;
    };
    if (! (mutex = calloc(qtty, sizeof(*mutex)))) {
        perror("Calloc for threead failed");
        return EXIT_FAILURE;
    };

    if (! (play = calloc(qtty, sizeof(*play)))) {
        perror("Calloc for threead failed");
        return EXIT_FAILURE;
    };

    for (int i = 0; i < qtty; ++i) {
        play[i] = false;
        if (pthread_mutex_init(mutex + i, nullptr) != 0) {
            perror("Pthread_mutex_init failed (should never happen)");
            return EXIT_FAILURE;
        }
        if (pthread_cond_init(cond + i, nullptr) != 0) {
            perror("Pthread_cond_init failed");
            return EXIT_FAILURE;
        }
    }

    // Initialize common variable...

    hit = a;

    // Start playing pingpong...

    play[0] = true;

    for (int i = 0; i < qtty; ++i) {
        if (pthread_create(thread + i, nullptr, thread_start, (void *) (intptr_t) i) != 0) {
            perror("Pthread_create failed");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < qtty; ++i) {
        if (pthread_join(*(thread + i), nullptr) != 0) {
            perror("Pthread_join failed");
            return EXIT_FAILURE;
        }
    }

    // Deallocating...

    free(play);
    free(mutex);
    free(cond);
    free(thread);

    return EXIT_SUCCESS;
}
