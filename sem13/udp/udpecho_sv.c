#include "udpecho.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int
main(int argc, char *argv[])
{

    if (argc < 3) {
        bye("Usage: ./udpecho_sv ip6_addr udp_port");
    }

    // Trying to fill sockaddr_in6 with user supplied ip6 address and port...

    struct sockaddr_in6 sv_addr;
    memset(&sv_addr, 0, sizeof(sv_addr));
    sv_addr.sin6_family = AF_INET6;
    // sv_addr.sin6_addr = in6addr_any;
    sv_addr.sin6_port = htons(strtol(argv[2], nullptr, 10));

    if (inet_pton(AF_INET6, argv[1], &sv_addr.sin6_addr) <= 0) {
        bye("inet_pton failed");
    }

    // Getting address and port back (to printf them later):
    // hopefully, they will become more presentable (at least ip6 address)...

    char sv_ntop[INET6_ADDRSTRLEN];
    int sv_port;

    if (inet_ntop(AF_INET6, &sv_addr.sin6_addr, sv_ntop, INET6_ADDRSTRLEN) != sv_ntop) {
        sv_ntop[0] = 0;
    }
    sv_port = ntohs(sv_addr.sin6_port);

    // Creating ipv6 udp socket...

    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);

    if (sockfd == -1) {
        bye("socket failed");
    }

    // Binding socket to address...

    if (bind(sockfd, (struct sockaddr *) &sv_addr, sizeof(sv_addr)) == -1) {
        bye("bind failed");
    }

    //

    for (;;) {

        // Creating buffer for incoming udp-datagram...

        char buf[ECHO_LENGTH];  // Not adding one byte to null-terminate string: we shall use %.*s printf precision specifier
        size_t bufsize = ECHO_LENGTH;

        // Creating address structure for incoming udp-datagram sender's address...

        struct sockaddr_in6 cl_addr;
        socklen_t addrlen = sizeof(cl_addr);

        // Receiving udp-datagram (this blocks)...

        ssize_t r = recvfrom(sockfd, buf, bufsize, 0, (struct sockaddr *) &cl_addr, &addrlen);

        if (r == -1) {
            bye("recvfrom failed");
        }

        // Making client's ip6-address and port host user friendly... 

        char cl_ntop[INET6_ADDRSTRLEN];
        int cl_port;

        if (inet_ntop(AF_INET6, &cl_addr.sin6_addr, cl_ntop, INET6_ADDRSTRLEN) != cl_ntop) {
            cl_ntop[0] = 0;
        }
        cl_port = ntohs(cl_addr.sin6_port);

        // No comments...

        printf("SV ip addr %s udp port %u received %ld bytes \"%.*s\" from CL ip addr %s udp port %u.\n",
            sv_ntop, sv_port,
            r, (int) r, buf,
            cl_ntop, cl_port);

        // Echoing udp-datagram back to client... MSG_DONTWAIT enables nonblocking operation:
        // if sendto would block, EAGAIN or EWOULDBLOCK is returned,
        // but we don't check for EAGAIN or EWOULDBLOCK, who cares about udp-datagrams...

        ssize_t s = sendto(sockfd, buf, r, MSG_DONTWAIT, (struct sockaddr *) &cl_addr, addrlen);

        if (s == -1) {
            bye("sendto failed");
        }

        // Only AI would seriously comment the next four lines...

        printf("SV ip addr %s udp port %u sent %ld bytes \"%.*s\" to CL ip addr %s udp port %u.\n",
            sv_ntop, sv_port,
            s, (int) s, buf,
            cl_ntop, cl_port);

    }

    // Neither instruction pointer, nor program counter should get here...

    if (close(sockfd) == -1) {
        bye("close failed");
    }

    return EXIT_SUCCESS;

}
