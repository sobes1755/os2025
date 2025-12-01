#define _POSIX_C_SOURCE 200112L  // getaddrinfo from <netdb.h>
#define	_GNU_SOURCE  // splice from <fcntl.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 512
#define FILENAME_LENGTH 256

[[noreturn]] static void
bye(char *msg)
{
    if (errno != 0) {
        fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, errno, strerror(errno));
    } else {
        fprintf(stderr, "%s!\n", msg);
    }

    exit(EXIT_FAILURE);
}

[[noreturn]] static void
bye_getaddrinfo(char *msg, int error)
{
    if (error != 0) {
        fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, error, gai_strerror(error));
    } else {
        fprintf(stderr, "%s!\n", msg);
    }

    exit(EXIT_FAILURE);
}

static void         
waitforzombies([[maybe_unused]] int sig)
{
    int errno_save = errno;
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
    }
    errno = errno_save;
}

int
main(int argc, char *argv[])
{

    if (argc < 3) {
        bye("Usage: ./tcp_epoll_sv sv_name sv_port sv_dirname");
    }

    // Creating hints to select suitable socket...

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);

    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    hints.ai_family = AF_UNSPEC;  // Both ipv4 and ipv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    struct addrinfo *result;

    int error = getaddrinfo(argv[1], argv[2], &hints, &result);

    if (error != 0) {
        bye_getaddrinfo("getaddrinfo failed", error);
    }

    // Creating socket... Binding scoket...

    int lsockfd;
    struct addrinfo *paddrinfo;    

    for (paddrinfo = result; paddrinfo != nullptr; paddrinfo = paddrinfo->ai_next) {

        lsockfd = socket(paddrinfo->ai_family, paddrinfo->ai_socktype, paddrinfo->ai_protocol);

        if (lsockfd == -1) {
            continue;
        }

        if (bind(lsockfd, paddrinfo->ai_addr, paddrinfo->ai_addrlen) == 0) {
            break;
        }

        close(lsockfd);

    }

    if (paddrinfo == nullptr) {
        bye("Socket/connect failed");
    }

    // Getting server address and listening port for printf....

    char lsv_ntop[INET6_ADDRSTRLEN];
    int lsv_port;

    void *lsv_paddr;

    if (paddrinfo->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *) paddrinfo->ai_addr;
        lsv_paddr = &(ipv4->sin_addr);
        lsv_port = ntohs(ipv4->sin_port);
    } else if (paddrinfo->ai_family == AF_INET6) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) paddrinfo->ai_addr;
        lsv_paddr = &(ipv6->sin6_addr);
        lsv_port = ntohs(ipv6->sin6_port);
    }

    inet_ntop(paddrinfo->ai_family, lsv_paddr, lsv_ntop, sizeof(lsv_ntop));

    //

    freeaddrinfo(result);

    // Ignore broken client pipes..

    signal(SIGPIPE, SIG_IGN);

    // Take care for zombies....

    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = waitforzombies;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        bye("sigaction failed");
    }

    // Starting to listen for connections...

    if (listen(lsockfd, BACKLOG) == -1) {
        bye("listen failed");
    }

    for (;;) {

        printf("SV %s %d: listening...\n", lsv_ntop, lsv_port);

        // Accepting client...

        struct sockaddr_in addr;
        socklen_t addrlen = sizeof addr;

        int sockfd = accept(lsockfd, (struct sockaddr *) &addr, (socklen_t *) &addrlen);

        if (sockfd == -1) {
            perror("accept failed");
            continue;
        }

        // Creating child to handkle client request...

        pid_t pid = fork();

        if (pid == -1) {

            perror("fork failed");
            close(sockfd);
            continue;

        } else if (pid > 0) {

            close(sockfd);
            continue;

        }

        // Child...

        close(lsockfd);

        // Getting server address and sending port (for printf)...

        char sv_ntop[INET6_ADDRSTRLEN];
        int sv_port;

        struct sockaddr_storage sv_addr;
        socklen_t sv_addrlen = sizeof(sv_addr);

        void *sv_paddr;

        if (getsockname(sockfd, (struct sockaddr *) &sv_addr, &sv_addrlen) == -1) {
            bye("getsockname failed");
        }

        if (sv_addr.ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) &sv_addr;
            sv_paddr = &(ipv4->sin_addr);
            sv_port = ntohs(ipv4->sin_port);
        } else if (sv_addr.ss_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&sv_addr;
            sv_paddr = &(ipv6->sin6_addr);
            sv_port = ntohs(ipv6->sin6_port);
        }

        inet_ntop(sv_addr.ss_family, sv_paddr, sv_ntop, sizeof(sv_ntop));

        // Getting client address and receivinf port (for printf)...

        char cl_ntop[INET6_ADDRSTRLEN];
        int cl_port;

        struct sockaddr_storage cl_addr;
        socklen_t cl_addrlen = sizeof(cl_addr);

        void *cl_paddr;

        if (getpeername(sockfd, (struct sockaddr *) &cl_addr, &cl_addrlen) == -1) {
            bye("getsockname failed");
        }

        if (cl_addr.ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) &cl_addr;
            cl_paddr = &(ipv4->sin_addr);
            cl_port = ntohs(ipv4->sin_port);
        } else if (cl_addr.ss_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&cl_addr;
            cl_paddr = &(ipv6->sin6_addr);
            cl_port = ntohs(ipv6->sin6_port);
        }

        inet_ntop(cl_addr.ss_family, cl_paddr, cl_ntop, sizeof(cl_ntop));

        printf("SV %s %d CL %s %d: accepted.\n", sv_ntop, sv_port, cl_ntop, cl_port);

        // Readig filename...

        char filename[FILENAME_LENGTH];

        ssize_t r = read(sockfd, filename, FILENAME_LENGTH - 1);

        if (r <= 0) {
            close(sockfd);
            bye("read failed");
        }

        filename[r] = '\0';

        // Making fullname....

        char fullname[2 * FILENAME_LENGTH];

        snprintf(fullname, 2 * FILENAME_LENGTH, "%s/%s", argv[3], filename);

        // Opening file...

        int fd = open(fullname, O_RDONLY);

        if (fd < 0) {
            close(sockfd);
            bye("Error opening file");
        }

        // Sending file...

        struct stat filestat;

        if (fstat(fd, &filestat) < 0) {
            close(fd);
            close(sockfd);
            bye("fstat error");
        }

        size_t filesize = filestat.st_size;

        printf("SV %s %d CL %s %d: %s -> %s (%ld bytes) started.\n", sv_ntop, sv_port, cl_ntop, cl_port, fullname, filename, filesize);

        ssize_t w = 1;

        while (w > 0) {
            w = sendfile(sockfd, fd, nullptr, filesize);
        }

        if (w < 0) {
            printf("SV %s %d CL %s %d: %s -> %s (%ld bytes) failed.\n", sv_ntop, sv_port, cl_ntop, cl_port, fullname, filename, filesize);
        } else {
            printf("SV %s %d CL %s %d: %s -> %s (%ld bytes) finished.\n", sv_ntop, sv_port, cl_ntop, cl_port, fullname, filename, filesize);
        }

        // Closing...

        close(fd);

        // Closing...

        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);

        // Child...

        _exit(EXIT_SUCCESS);

    }

    // Shutting the connection and closing socket...

    shutdown(lsockfd, SHUT_RDWR);
    close(lsockfd);

    return EXIT_SUCCESS;

}
