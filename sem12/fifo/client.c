#include "readwrite.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int
main(int argc, char *argv[])
{
    if (argc < 3) {
        bye("Usage: client mul1 mul2 [FIFO]");
    }

    char *mul1 = argv[1];
    char *mul2 = argv[2];

    char *sfifo = (argc > 3)? argv[3]: SERVER_FIFO;

    // Creating client's FIFO...

    char cfifo[CLIENT_FIFO_LENGTH];
    snprintf(cfifo, sizeof(cfifo), CLIENT_FIFO, getpid());

    mode_t umask_save = umask(0);
    if ((mkfifo(cfifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1) && errno != EEXIST) {
        bye("Client: mkfifo failed");
    }
    umask(umask_save);

    printf("Client (%d): created client's FIFO (\"%s\").\n\n", getpid(), cfifo);

    // Openning server's FIFO...

    int sfd1 = open(sfifo, O_WRONLY);

    if (sfd1 == -1) {
        bye("Client: server's FIFO open failed");
    }

    printf("Client (%d): opened server's FIFO (\"%s\").\n", getpid(), sfifo);

    // Writing client's header ro server's FIFO...

    struct client_header ch = {};

    ch.pid = getpid();
    ch.size1 = strlen(mul1);
    ch.size2 = strlen(mul2);

    if (write(sfd1, &ch, sizeof(ch)) != sizeof(ch)) {
        bye("Client: write of client's header failed");
    }

    printf("Client (%d): wrote client's header (pid = %d, size1 = %zu, size2 = %zu) to server's FIFO.\n",
        getpid(), ch.pid, ch.size1, ch.size2);

    // Writing multipliers to server's FIFO...

    if (write(sfd1, mul1, ch.size1) != (ssize_t) ch.size1) {
        bye("Client: write of mul1 failed");
    }

    printf("Client (%d): wrote mul1 (\"%s\") to server's FIFO.\n",
        getpid(), mul1);

    if (write(sfd1, mul2, ch.size2) != (ssize_t) ch.size2) {
        bye("Client: write of mul2 failed");
    }

    printf("Client (%d): wrote mul2 (\"%s\") to server's FIFO.\n",
        getpid(), mul2);

    // Closing server's FIFO....

    if (close(sfd1) == -1) {
        bye("Client: server's FIFO close failed");
    }

    printf("Client (%d): closed server's FIFO (\"%s\").\n\n", getpid(), sfifo);

    // Opening client's FIFO...

    printf("Client (%d): opening client's FIFO (\"%s\")...\n", getpid(), cfifo);

    int cfd0 = open(cfifo, O_RDONLY);

    if (cfd0 == -1) {
        bye("Client: open O_RDONLY failed");
    }

    printf("Client (%d): ...opened client's FIFO.\n", getpid());

    // Reading servers's header from client's FIFO..

    struct server_header sh = {};

    printf("Client (%d): reading server's header...\n", getpid());

    if (read(cfd0, &sh, sizeof(sh)) != sizeof(sh)) {
        bye("Client: server's header read failed");
    }

    printf("Client (%d): ...read server's header (size = %zu).\n", getpid(), sh.size);

    // Reading product from client's FIFO..

    char *product = calloc(sh.size, sizeof(*product));

    printf("Client (%d): reading product...\n", getpid());

    if (read(cfd0, product, sh.size) != (ssize_t) sh.size) {
        bye("Client: product read failed");
    }

    printf("Client (%d): ...read product (\"%s\").\n", getpid(), product);

    free(product);

    // Closing client's FIFO...

    if (close(cfd0) == -1) {
        bye("Client: client's FIFO close failed");
    }

    printf("Client (%d): closed client's FIFO (\"%s\").\n\n", getpid(), cfifo);

    // Deleting client's FIFO...

    if (unlink(cfifo) == -1) {
        bye("Client: client's FIFO unlink failed");
    };

    printf("Client (%d): deleted client's FIFO (\"%s\").\n", getpid(), cfifo);

    exit(EXIT_SUCCESS);

}
