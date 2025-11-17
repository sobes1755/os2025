#include "readwrite.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>    
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>

int
main(int argc, char *argv[])
{
    if (argc < 3) {
        bye("Usage: client mul1 mul2");
    }

    char *mul1 = argv[1];
    char *mul2 = argv[2];

    size_t size1 = strlen(mul1) + 1;  // + 1 for '\0'
    size_t size2 = strlen(mul2) + 1;  // + 1 for '\0'

    // Creating client's SHMEM...

    size_t shmsize = size1 + size2;

    int shmid = shmget(IPC_PRIVATE, shmsize,
        IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    if (shmid == -1) {
        bye("Client: shmget failed");
    }

    printf("Client (%d): created SHMEM (%d).\n", getpid(), shmid);

    // Attaching client's SHMEM...

    char *shmptr = shmat(shmid, nullptr, 0);

    if (shmptr == (void *) -1) {
        bye("Client: shmat failed");
    }

    printf("Client (%d): attached SHMEM (%p).\n\n", getpid(), shmptr);

    // Writing multipliers to SHMEM...

    memcpy(shmptr + 0, mul1, size1);
    memcpy(shmptr + size1, mul2, size2);

    printf("Client (%d): wrote multipliesr to client's SHMEM ", getpid());
    printf("(mul1 = \"%s\", ", shmptr);
    printf("mul2 = \"%s\").\n\n", shmptr + size1);

    // Creating server's MSG key...

    key_t msgkey = ftok(SERVER_NAME, 1);

    if (msgkey == (key_t) -1) {
        bye("Ftok failed!");
    }

    printf("Client (%d): created MSG key (%#x).\n", getpid(), msgkey);

    // Opening server's MSGQ...

    int msgid = msgget(msgkey, S_IRUSR | S_IWUSR);

    if (msgid == -1) {
        bye("Client: msgget failed");
    }

    printf("Client (%d): opened MSG queue (%d).\n", getpid(), msgid);

    // Sending client's message ro server's MSG queue...

    struct client_message cm = {};

    cm.mtype = 1;
    cm.pid = getpid();
    cm.shmid = shmid;
    cm.size1 = size1;
    cm.size2 = size2;

    printf("Client (%d): sending MSG (mtype = %ld, shmid = %d, size1 = %zu, size2 = %zu) to server's MSG queue...\n",
        getpid(), cm.mtype, cm.shmid, cm.size1, cm.size2);

    if (msgsnd(msgid, &cm, client_message_size, 0) == -1) {
        bye("Client: msgsnd failed");
    }

    printf("Client (%d): ...sent MSG to server's MSG queue.\n", getpid());

    // Receiving servers's message from server's MSG queue...

    struct server_message sm = {};

    printf("Client (%d): receiving MSG from server's MSG queue...\n", getpid());

    if (msgrcv(msgid, &sm, server_message_size, getpid(), 0) != server_message_size) {
        bye("Client: msgrcv failed");
    }

    printf("Client (%d): ...received MSG (mtype = %ld, size = %zu).\n\n",
        getpid(), sm.mtype, sm.size);

    // Reading product from client's SHMEM...

    char *product = shmptr;

    printf("Client (%d): read product from client's SHMEM (\"%s\").\n\n", getpid(), product);

    // Detaching client's SHMEM...

    if (shmdt(shmptr) == -1) {
        bye("Client: shmdt failed");
    }

    printf("Client (%d): detached SHMEM (%p).\n", getpid(), shmptr);

    // Deleting client's SHMEM...

    if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
        bye("Client: delete SHMEM failed");
    };

    printf("Client (%d): removed SHMEM (%d).\n", getpid(), shmid);

    exit(EXIT_SUCCESS);

}
