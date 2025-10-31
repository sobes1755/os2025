/*

NAME

spsc -- experiment with pthreads and producer-consumer problem with single producer and single consumer

SYNOPSIS

spsc

DESCRIPTION

The producer sends a message to the consumer,
waits for the consumer to read the message and sends the next message.
The consumer waits for the message,
reads the message and waits for the next message.

*/

#include "gettime.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

static enum {READY_TO_WRITE, READY_TO_READ} state = READY_TO_WRITE;
static char var = {};
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void
spsc_syn_send(char msg)
{
    if (pthread_mutex_lock(&mutex) != 0) {
        perror("Send: pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    while (state != READY_TO_WRITE) {
        if (pthread_cond_wait(&cond, &mutex) != 0) {
            perror("Send: pthread_cond_wait failed");
            exit(EXIT_FAILURE);
        }
    }

    var = msg;
    printf("Producer: message '%c' at %f s sent.\n", msg, gettime_monotonic());
    state = READY_TO_READ;

    if (pthread_mutex_unlock(&mutex) != 0) {
        perror("Send: pthread_mutex_unlock failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_signal(&cond) != 0) {
        perror("Send: pthread_cond_signal failed");
        exit(EXIT_FAILURE);
    }

    return;
}

char
spsc_syn_recv(void)
{
    if (pthread_mutex_lock(&mutex) != 0) {
        perror("Recv: pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    while (state != READY_TO_READ) {
        if (pthread_cond_wait(&cond, &mutex) != 0) {
            perror("Send: pthread_cond_wait failed");
            exit(EXIT_FAILURE);
        }
    };

    char msg = var;
    printf("Consumer: message '%c' at %f s received.\n", msg, gettime_monotonic());
    state = READY_TO_WRITE;

    if (pthread_mutex_unlock(&mutex) != 0) {
        perror("Recv: pthread_mutex_unlock failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_signal(&cond) != 0) {
        perror("Recv: pthread_cond_signal failed");
        exit(EXIT_FAILURE);
    }

    return msg;
}

void *
p_start(void *)
{
    for (char i = 'A'; i <= 'Z'; ++i) {
        char msg = i;
        spsc_syn_send(msg);
    }

    return nullptr;
}

void *
c_start(void *)
{
    for (char i = 'A'; i <= 'Z'; ++i) {    
        char msg = spsc_syn_recv();
        assert(i = msg);
    }

    return nullptr;
}

int
main(void)
{

    pthread_t p;  // producer
    pthread_t c;  // consumer

    if (pthread_create(&p, nullptr, p_start, nullptr) != 0) {
        perror("Pthread_create failed");
        return EXIT_FAILURE;
    }
    if (pthread_create(&c, nullptr, c_start, nullptr) != 0) {
        perror("Pthread_create failed");
        return EXIT_FAILURE;
    }

    if (pthread_join(c, nullptr) != 0) {
        perror("Pthread_join failed");
        return EXIT_FAILURE;
    }
    if (pthread_join(p, nullptr) != 0) {
        perror("Pthread_join failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
