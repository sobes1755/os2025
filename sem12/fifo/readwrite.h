#pragma once

#define _POSIX_C_SOURCE 200809L  // fdopen, getline

#include <unistd.h>

#define SERVER_FIFO "./server.fifo"
#define SERVER_LENGTH sizeof(SERVER_FIFO)

#define CLIENT_FIFO "./client.fifo.%d"
#define CLIENT_FIFO_LENGTH (sizeof(CLIENT_FIFO) + 64)

struct client_header {
    pid_t pid;  // Client's pid
    size_t size1;  // Size of 1st multiplier
    size_t size2;  // Size of 2nd multiplier
};

struct server_header {
    size_t size;  // Size of product
};

void
writeline(int fd, char *buffer, size_t size);
ssize_t
readline(int fd, char **pbuffer, size_t *psize);

[[noreturn]] void
bye(char *msg);
