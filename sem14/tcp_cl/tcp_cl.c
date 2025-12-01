#define _POSIX_C_SOURCE 200112L  // getaddrinfo from <netdb.h>
#define	_GNU_SOURCE  // splice from <fcntl.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

[[noreturn]] void
bye(char *msg)
{
    if (errno != 0) {
        fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, errno, strerror(errno));
    } else {
        fprintf(stderr, "%s!\n", msg);
    }

    exit(EXIT_FAILURE);
}

[[noreturn]] void
bye_getaddrinfo(char *msg, int error)
{
    if (error != 0) {
        fprintf(stderr, "%s (errno = %d, errmsg = \"%s\")!\n", msg, error, gai_strerror(error));
    } else {
        fprintf(stderr, "%s!\n", msg);
    }

    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{

    if (argc < 5) {
        bye("Usage: ./tcp_cl sv_name sv_port file_sv file_cl file_size");
    }

    char *file_sv = argv[3];
    char *file_cl = argv[4];
    size_t file_size = strtol(argv[5], nullptr, 10);  // Do not receive more than file_size bytes

    // Looking for a server...

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

    // Connecting to server...

    int sockfd;
    struct addrinfo *paddrinfo;    

    for (paddrinfo = result; paddrinfo != nullptr; paddrinfo = paddrinfo->ai_next) {

        sockfd = socket(paddrinfo->ai_family, paddrinfo->ai_socktype, paddrinfo->ai_protocol);

        if (sockfd == -1) {
            continue;
        }

        if (connect(sockfd, paddrinfo->ai_addr, paddrinfo->ai_addrlen) != -1) {
            break;
        }

        close(sockfd);

    }

    if (paddrinfo == nullptr) {
        bye("Socket/connect failed");
    }

    // Getting server address and port for printf....

    char sv_ntop[INET6_ADDRSTRLEN];
    int sv_port;

    void *sv_paddr;

    if (paddrinfo->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *) paddrinfo->ai_addr;
        sv_paddr = &(ipv4->sin_addr);
        sv_port = ntohs(ipv4->sin_port);
    } else if (paddrinfo->ai_family == AF_INET6) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) paddrinfo->ai_addr;
        sv_paddr = &(ipv6->sin6_addr);
        sv_port = ntohs(ipv6->sin6_port);
    }

    inet_ntop(paddrinfo->ai_family, sv_paddr, sv_ntop, sizeof(sv_ntop));

    //

    freeaddrinfo(result);

    // Getting client address and port for printf (they are available after connect)...

    char cl_ntop[INET6_ADDRSTRLEN];
    int cl_port;

    struct sockaddr_storage cl_addr;
    socklen_t cl_addrlen = sizeof(cl_addr);

    void *cl_paddr;

    if (getsockname(sockfd, (struct sockaddr *) &cl_addr, &cl_addrlen) == -1) {
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

    // Sending filename to server...

    ssize_t w = write(sockfd, file_sv, strlen(file_sv));

    if (w != (ssize_t) strlen(file_sv)) {
        bye("write failed");
    }

    shutdown(sockfd, SHUT_WR);

    // Receiving file from server...

    int fd = open(file_cl, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1) {
        bye("open failed");
    }

    // Using Linux splice system call to write from pipe to pipe...

    int pipefd[2];

    if (pipe(pipefd) == -1) {
        bye("pipe failed");
    }

    //

    printf("CL %s %d SV %s %d: %s -> %s started.\n", cl_ntop, cl_port, sv_ntop, sv_port, file_sv, file_cl);

    for (;;) {

        ssize_t w1 = splice(sockfd, nullptr, pipefd[1], nullptr, file_size, SPLICE_F_MOVE | SPLICE_F_MORE);

        printf("CL %s %d SV %s %d: read from socket = %ld\n", cl_ntop, cl_port, sv_ntop, sv_port, w1);

        if (w1 == -1) {
            bye("splice (sockfd -> pipefd[1]) failed");
        }

        if (w1 == 0) {
            break;
        }       

        ssize_t w2 = splice(pipefd[0], nullptr, fd, nullptr, strtol(argv[5], nullptr, 10), 0);

        printf("CL %s %d SV %s %d: written to file = %ld\n", cl_ntop, cl_port, sv_ntop, sv_port, w2);

        if (w2 == -1) {
            bye("splice (pipefd[0] -> fd) failed");
        }

        file_size = file_size - w1; 

    }

    //

    printf("CL %s %d SV %s %d: %s -> %s finished.\n", cl_ntop, cl_port, sv_ntop, sv_port, file_sv, file_cl);

    //

    if (close(pipefd[1]) == -1) {
        bye("close pipefd[1]");
    }

    if (close(pipefd[0]) == -1) {
        bye("close pipefd[0]");
    }

    // Closing file...

    if (close(fd) == -1) {
        bye("close failed");
    }

    // Shutting the connection and closing socket...

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    return EXIT_SUCCESS;

}
