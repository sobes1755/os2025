/*

NAME

getaddrinfo -- prints a list of socket address structures given a host name and a service name 

SYNOPSIS

Usage: ./getaddrinfo
  -h host
  -s service
  -f AF_INET|AF_INET6|AF_UNSPEC
  -t SOCK_DGRAM|SOCK_STREAM
  -o AI_ADDRCONFIG
  -o AI_ALL
  -o AI_CANONNAME
  -o AI_NUMERICHOST
  -o AI_NUMERICSERV
  -o AI_PASSIVE
  -o AI_V4MAPPED

DESCRIPTION

./getaddrinfo -h localhost -s https -f AF_INET6 -t SOCK_STREAM

ai_flags = 0
ai_family = 10 (AF_INET = 2, AF_INET6 = 10)
ai_socktype = 1 (SOCK_STREAM = 1, SOCK_DGRAM = 2, SOCK_RAW = 3)
ai_protocol = 6
ai_canonname = (null)
ai_addrlen = 28
ai_addr = 0x5bc5c7e738b0

ai_addr->sin6_family = 10
ntohs(ai_addr->sin6_port) = 443
inet_ntop(ai_family, ai_addr->sin6_addr) = ::1

*/

#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

[[noreturn]] void
bye_usage()
{
    printf("Usage: ./getaddrinfo -h localhost -f AF_INET -o AI_PASSIVE\n");
    printf("-h host\n");
    printf("-s service\n");
    printf("-f AF_INET|AF_INET6|AF_UNSPEC\n");
    printf("-t SOCK_DGRAM|SOCK_STREAM\n");
    printf("-o AI_ADDRCONFIG\n");
    printf("-o AI_ALL\n");
    printf("-o AI_CANONNAME\n");
    printf("-o AI_NUMERICHOST\n");
    printf("-o AI_NUMERICSERV\n");
    printf("-o AI_PASSIVE\n");
    printf("-o AI_V4MAPPED\n");

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

    char *host = nullptr;
    char *service = nullptr;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;

    int opt = 0;

    while ((opt = getopt(argc, argv, ":h:s:f:t:o:")) != -1) {
        switch (opt) {
        case 'h':
            host = optarg;
            break;
        case 's':
            service = optarg;
            break;
        case 'f':
            if (strcmp(optarg, "AF_INET") == 0) {
                hints.ai_family = AF_INET;
                break;
            } else if (strcmp(optarg, "AF_INET6") == 0) {
                hints.ai_family = AF_INET6;
                break;
            } else if (strcmp(optarg, "AF_UNSPEC") == 0) {
                hints.ai_family = AF_UNSPEC;
                break;
            } else {
                bye_usage();
            }
        case 't':
            if (strcmp(optarg, "SOCK_DGRAM") == 0) {
                hints.ai_socktype = SOCK_DGRAM;
                break;
            } else if (strcmp(optarg, "SOCK_STREAM") == 0) {
                hints.ai_socktype = SOCK_STREAM;
                break;
            } else {
                bye_usage();
            }
        case 'o':
            if (strcmp(optarg, "AI_ADDRCONFIG") == 0) {
                hints.ai_flags |= AI_ADDRCONFIG;
                break;
            } else if (strcmp(optarg, "AI_ALL") == 0) {
                hints.ai_flags |= AI_ALL;
                break;
            } else if (strcmp(optarg, "AI_CANONNAME") == 0) {
                hints.ai_flags |= AI_CANONNAME;
                break;
            } else if (strcmp(optarg, "AI_NUMERICHOST") == 0) {
                hints.ai_flags |= AI_NUMERICHOST;
                break;
            } else if (strcmp(optarg, "AI_NUMERICSERV") == 0) {
                hints.ai_flags |= AI_NUMERICSERV;
                break;
            } else if (strcmp(optarg, "AI_PASSIVE") == 0) {
                hints.ai_flags |= AI_PASSIVE;
                break;
            } else if (strcmp(optarg, "AI_V4MAPPED") == 0) {
                hints.ai_flags |= AI_V4MAPPED;
                break;
            } else {
                bye_usage();
            }
        default:
            bye_usage();
        }
    }

    struct addrinfo *result;

    int error = getaddrinfo(host, service, &hints, &result);

    if (error != 0) {
        bye_getaddrinfo("getaddrinfo failed", error);
    }

    for (auto paddrinfo = result; paddrinfo != nullptr; paddrinfo = paddrinfo->ai_next) {

        printf("ai_flags = %d\n", paddrinfo->ai_flags);
        printf("ai_family = %d (AF_INET = %d, AF_INET6 = %d)\n", paddrinfo->ai_family, AF_INET, AF_INET6);
        printf("ai_socktype = %d (SOCK_STREAM = %d, SOCK_DGRAM = %d, SOCK_RAW = %d)\n", paddrinfo->ai_socktype, SOCK_STREAM, SOCK_DGRAM, SOCK_RAW);
        printf("ai_protocol = %d\n", paddrinfo->ai_protocol);
        printf("ai_canonname = %s\n", paddrinfo->ai_canonname);
        printf("ai_addrlen = %d\n", paddrinfo->ai_addrlen);
        printf("ai_addr = %p\n\n", paddrinfo->ai_addr);

        if (paddrinfo->ai_family == AF_INET) {

            char present[INET_ADDRSTRLEN];

            if (inet_ntop(paddrinfo->ai_family, &(((struct sockaddr_in *)(paddrinfo->ai_addr))->sin_addr), present, INET_ADDRSTRLEN) != present) {
                bye("inet_ntop failed");
            }

            printf("ai_addr->sin_family = %d\n", ((struct sockaddr_in *)(paddrinfo->ai_addr))->sin_family);
            printf("ntohs(ai_addr->sin_port) = %d\n", ntohs(((struct sockaddr_in *)(paddrinfo->ai_addr))->sin_port));
            printf("inet_ntop(ai_family, ai_addr->sin_addr) = %s\n\n", present);            

        } else if (paddrinfo->ai_family == AF_INET6) {
        
            char present[INET6_ADDRSTRLEN];

            if (inet_ntop(paddrinfo->ai_family, &(((struct sockaddr_in6 *)(paddrinfo->ai_addr))->sin6_addr), present, INET6_ADDRSTRLEN) != present) {
                bye("inet_ntop failed");
            }

            printf("ai_addr->sin6_family = %d\n", ((struct sockaddr_in6 *)(paddrinfo->ai_addr))->sin6_family);
            printf("ntohs(ai_addr->sin6_port) = %d\n", ntohs(((struct sockaddr_in6 *)(paddrinfo->ai_addr))->sin6_port));
            printf("inet_ntop(ai_family, ai_addr->sin6_addr) = %s\n\n", present);            

        }

    }

    freeaddrinfo(result);

    return EXIT_SUCCESS;

}
