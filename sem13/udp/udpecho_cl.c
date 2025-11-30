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

    const int MAGICNUMBER = 3;

    if (argc < MAGICNUMBER + 1) {
        bye("Usage: ./udpecho_cl ip6_addr udp_port msg1 msg2...");
    }

    // Trying to fill sockaddr_in6 structure with server's ip6 address and udp port...

    struct sockaddr_in6 sv_addr;

    memset(&sv_addr, 0, sizeof(sv_addr));
    sv_addr.sin6_family = AF_INET6;
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

    // Binding client socket just to call getsockname
    // just to get client address and port (to printf them later in their beauty)...

    struct sockaddr_in6 cl_addr;
    memset(&cl_addr, 0, sizeof(cl_addr));
    cl_addr.sin6_family = AF_INET6;
    cl_addr.sin6_addr = in6addr_any;
    cl_addr.sin6_port = 0;  // Free ephemeral port will be selected

    if (bind(sockfd, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1) {
        bye("bind failed");
    }

    socklen_t cl_addrlen = sizeof(cl_addr);

    if (getsockname(sockfd, (struct sockaddr *) &cl_addr, &cl_addrlen) == -1) {
        bye("getsockname failed");
    }

    char cl_ntop[INET6_ADDRSTRLEN];
    int cl_port;

    if (inet_ntop(AF_INET6, &cl_addr.sin6_addr, cl_ntop, INET6_ADDRSTRLEN) != cl_ntop) {
        cl_ntop[0] = 0;
    }
    cl_port = ntohs(cl_addr.sin6_port);

    // Sending (argc - MAGICNUMBER + 1) udp datagrams to server....

    for (int i = MAGICNUMBER; i < argc; ++i) {

        // Sending... Should I have commented this?...

        ssize_t s = sendto(sockfd, argv[i], strlen(argv[i]), 0, (struct sockaddr *) &sv_addr, sizeof(sv_addr));

        if (s == -1) {
            bye("sendto failed");
        }

        // Printing report...

        printf("CL ip addr %s udp port %u sent %ld bytes \"%.*s\" to SV ip addr %s udp port %u.\n",
            cl_ntop, cl_port,
            s, (int) s, argv[i],
            sv_ntop, sv_port);

        // Receiving echo... Or not receiving and waiting forever here...

        char buf[ECHO_LENGTH];

        struct sockaddr_in6 peer_addr;
        socklen_t peer_addrlen = sizeof(peer_addr);
        char peer_ntop[INET6_ADDRSTRLEN];
        int peer_port;

        ssize_t r = recvfrom(sockfd, buf, ECHO_LENGTH, 0, (struct sockaddr *) &peer_addr, &peer_addrlen);

        if (r == -1) {
            bye("recvfrom failed");
        }

        if (inet_ntop(AF_INET6, &peer_addr.sin6_addr, peer_ntop, INET6_ADDRSTRLEN) != peer_ntop) {
            peer_ntop[0] = 0;
        }
        peer_port = ntohs(peer_addr.sin6_port);

        // Printing report...

        printf("CL ip addr %s udp port %u received %ld bytes \"%s\" from SV ip addr %s udp port %u.\n",
            cl_ntop, cl_port,
            r, buf,
            peer_ntop, peer_port);

    }

    // Neither instruction pointer, nor program counter should get here...

    if (close(sockfd) == -1) {
        bye("close failed");
    }

    // But if one of them gets...

    return EXIT_SUCCESS;

}
