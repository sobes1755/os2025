#define _GNU_SOURCE

#include <sys/signalfd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

[[noreturn]] void
bye(char *msg)
{
    if (errno > 0) {
        fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, errno, strerror(errno));
    } else {
        fprintf(stderr, "%s!\n", msg);
    }

    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{

    if (argc < 2) {
        bye("Usage: signalfd signum1 signum2...");
    }

    // Generating signal mask....

    sigset_t mask;
    sigemptyset(&mask);

    for (int i = 1; i < argc; ++i) {
        sigaddset(&mask, strtol(argv[i], nullptr, 10));
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        bye("Siprocmask failed");
    }

    // Openning signal file descriptor...

    int fd = signalfd(-1, &mask, 0);

    if (fd == -1) {
        bye("Signalfd failed!");
    }

    printf("Process %d is ready to catch signals!\n", getpid());

    for (;;) {

        struct signalfd_siginfo s;

        ssize_t r = read(fd, &s, sizeof(s));

        if (r != sizeof(s)) {
            bye("Read failed");
        }

        printf("Process %d caught signal:\n\tssi_signo = %d (\'%s\'),\n\tssi_code = %d (",
            getpid(), s.ssi_signo, strsignal(s.ssi_signo), s.ssi_code);

        switch (s.ssi_code) {
        case SI_ASYNCIO:
            printf("%s", "SI_ASYNCIO");
            break;
        case SI_KERNEL:
            printf("%s", "SI_KERNEL");
            break;
        case SI_MESGQ:
            printf("%s", "SI_MESGQ");
            break;
        case SI_QUEUE:
            printf("%s", "SI_QUEUE");
            break;
        case SI_TIMER:
            printf("%s", "SI_TIMER");
            break;
        case SI_TKILL:
            printf("%s", "SI_TKILL");
            break;
        case SI_USER:
            printf("%s", "SI_USER");
            break;
        default:
            printf("%s", "SI_UNKNOWN");
            break;
        }

        printf("),\n\tssi_pid = %d, ssi_uid = %d,\n\tssi_int = %d, ssi_ptr = %p.\n",
            s.ssi_pid, s.ssi_uid, s.ssi_int, (void *) s.ssi_ptr);

    }

    close(fd);

    return EXIT_SUCCESS;

}
