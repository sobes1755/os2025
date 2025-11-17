#pragma once

#define _POSIX_C_SOURCE 200809L  // fdopen, getline

#include <unistd.h>

#define SERVER_NAME "./server"
#define SERVER_LENGTH sizeof(SERVER_NAME)

#define CLIENT_NAME "./client.fifo.%d"
#define CLIENT_NAME_LENGTH (sizeof(CLIENT_NAME) + 64)

struct client_message {
    long mtype;
    int pid;  // Client's pid
    int shmid;  // Client's shmid
    size_t size1;  // Size of 1st multiplier
    size_t size2;  // Size of 2nd multiplier
};

struct server_message {
    long mtype;
    size_t size;  // Size of product
};

#define client_message_size (sizeof(struct client_message) - sizeof(long))
#define server_message_size (sizeof(struct server_message) - sizeof(long))

void
writeline(int fd, char *buffer, size_t size);
ssize_t
readline(int fd, char **pbuffer, size_t *psize);

[[noreturn]] void
bye(char *msg);
