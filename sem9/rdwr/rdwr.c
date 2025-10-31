/*

NAME

rdwr -- experiment with pthreads and readers-writers problem

SYNOPSIS

rdwr

DESCRIPTION

A pair of writers writes chars to buffer.
A pair of readers reads chars from buffer.

To eliminate race conditions pthread_rwlock_t rwlock are used
(or aren't used if WITHOUT_RW_LOCK is defined).

TODO: check the return values of pthread_* functions.

*/

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum {SIZE = 8 * 120 + 1};
static char buffer[SIZE] = {};

#ifndef WITHOUT_RW_LOCK
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
#endif

void *
wr_start(void *)
{
    for (;;) {

        char c = rand();

        if ((('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z'))) {

#ifndef WITHOUT_RW_LOCK
            pthread_rwlock_wrlock(&rwlock);
#endif
            for (int i = 0; i < SIZE - 1; ++i) {
                buffer[i] = c;
            }
#ifndef WITHOUT_RW_LOCK
            pthread_rwlock_unlock(&rwlock);
#endif

            usleep(1E3 * rand() / RAND_MAX);

        }

    }

    return nullptr;
}

void *
rd_start(void *)
{
    for (int count = 0; count < 8; ++count) {

#ifndef WITHOUT_RW_LOCK
        pthread_rwlock_rdlock(&rwlock);    
#endif
        printf("%s\n\n", buffer);
#ifndef WITHOUT_RW_LOCK
        pthread_rwlock_unlock(&rwlock);
#endif

        usleep(1E6 * rand() / RAND_MAX);

    }

    return nullptr;
}

int
main(void)
{
    pthread_t wr1;  // writer 1
    pthread_t wr2;  // writer 2
    pthread_t rd1;  // reader 1
    pthread_t rd2;  // reader 2

    pthread_create(&wr1, nullptr, wr_start, nullptr);
    pthread_create(&wr2, nullptr, wr_start, nullptr);
    pthread_create(&rd1, nullptr, rd_start, nullptr);
    pthread_create(&rd2, nullptr, rd_start, nullptr);

    pthread_join(rd2, nullptr);  // Do not wait for writers to finish,
    pthread_join(rd1, nullptr);  // exiting as soon as readers are tired.
    // pthread_join(wr2, nullptr);
    // pthread_join(wr1, nullptr);

    return EXIT_SUCCESS;
}
