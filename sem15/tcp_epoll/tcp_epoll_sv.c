#define _POSIX_C_SOURCE 200112L  // getaddrinfo from <netdb.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define BACKLOG 1024
#define EPOLL_EVENTS_SIZE 256
#define READ_BUFFER_SIZE 65536

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

void
get_ntop_port(struct sockaddr *paddr, char *ntop, int *port)
{
    void *temp;

    if (paddr->sa_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *) paddr;
        temp = &(ipv4->sin_addr);
        *port = ntohs(ipv4->sin_port);
    } else if (paddr->sa_family == AF_INET6) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) paddr;
        temp = &(ipv6->sin6_addr);
        *port = ntohs(ipv6->sin6_port);
    }

    inet_ntop(paddr->sa_family, temp, ntop, INET6_ADDRSTRLEN);
}

void
set_socket_nonblock(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL);

    if (flags == -1) {
        bye("fcntl F_GETFL failed");
        exit(EXIT_FAILURE);
    }

    flags = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    if (flags == -1) {
        bye("fcntl F_SETFL failed");
        exit(EXIT_FAILURE);
    }
}

struct epoll_event_data {
    int fd;
    char ntop[INET6_ADDRSTRLEN];
    int port;    
};

void
add_socket_to_epoll(int epollfd, int sockfd, struct epoll_event_data *pdata)
{
    struct epoll_event event = {.events = EPOLLIN | EPOLLET, .data.ptr = pdata,};

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
        bye("epoll_ctl EPOLL_CTL_ADD failed");
    }
}

void
del_socket_from_epoll(int epollfd, int sockfd)
{
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, nullptr) == -1) {
        bye("epoll_ctl EPOLL_CTL_DEL failed");
    }
}

volatile sig_atomic_t terminated;

static void
handler([[maybe_unused]] int sig)
{
    terminated = 1;
}

int
main(int argc, char *argv[])
{
    if (argc < 3) {
        bye("Usage: ./tcp_epoll_sv sv_name sv_port");
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

    int listsockfd;

    struct addrinfo *paddrinfo;    

    for (paddrinfo = result; paddrinfo != nullptr; paddrinfo = paddrinfo->ai_next) {

        listsockfd = socket(paddrinfo->ai_family, paddrinfo->ai_socktype, paddrinfo->ai_protocol);

        if (listsockfd == -1) {
            continue;
        }

        if (bind(listsockfd, paddrinfo->ai_addr, paddrinfo->ai_addrlen) == 0) {
            break;
        }

        close(listsockfd);

    }

    if (paddrinfo == nullptr) {
        bye("socket/connect failed");
    }

    // Getting server address and listening port for printf....

    char sv_ntop[INET6_ADDRSTRLEN];
    int sv_port;

    get_ntop_port(paddrinfo->ai_addr, sv_ntop, &sv_port);    

    //

    freeaddrinfo(result);

    // Ignore broken client pipes..

    signal(SIGPIPE, SIG_IGN);

    // Terminating nicely...

    struct sigaction sa = {.sa_handler = handler,};
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        bye("sigaction SIGINT failed");
    }
    if (sigaction(SIGQUIT, &sa, nullptr) == -1) {
        bye("sigaction SIGQUIT failed");
    }

    // Create epoll instance...

    int epollfd = epoll_create1(EPOLL_CLOEXEC);

    if (epollfd == -1) {
        bye("epoll_create1 failed");
    }

    // Add the listening socket to the epoll instance...

    set_socket_nonblock(listsockfd);

    struct epoll_event_data listsockdata;

    listsockdata.fd = listsockfd;

    add_socket_to_epoll(epollfd, listsockfd, &listsockdata);

    // Starting to listen for connections...

    if (listen(listsockfd, BACKLOG) == -1) {
        bye("listen failed");
    }

    printf("SV %s %d: socket, bind, epoll_create, listen.\n", sv_ntop, sv_port);

    while (! terminated) {

        printf("SV %s %d: polling...\n", sv_ntop, sv_port);

        struct epoll_event events[EPOLL_EVENTS_SIZE];

        int ready = epoll_wait(epollfd, events, EPOLL_EVENTS_SIZE, -1);

        if ((ready == -1) && (errno != EINTR)) {
            bye("epoll_wait failed");
        }

        for (int i = 0; i < ready; ++i) {

            struct epoll_event_data *pdata = (struct epoll_event_data *) events[i].data.ptr;

            if (pdata->fd == listsockfd) {

                // News on listening socket!

                struct sockaddr_storage addr;
                socklen_t addrlen = sizeof addr;

                int sockfd;

                while ((sockfd = accept(listsockfd, (struct sockaddr *) &addr, &addrlen)) != -1) {

                    struct epoll_event_data *pnewdata = malloc(sizeof(struct epoll_event_data));

                    if (pnewdata == nullptr) {
                        bye("calloc failed");
                    }

                    set_socket_nonblock(sockfd);
                    pnewdata->fd = sockfd;
                    get_ntop_port((struct sockaddr *) &addr, pnewdata->ntop, &pnewdata->port);
                    add_socket_to_epoll(epollfd, sockfd, pnewdata);

                    printf("SV %s %d CL %s %d: accepted.\n", sv_ntop, sv_port, pnewdata->ntop, pnewdata->port);

                }

                if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                    bye("accept failed");
                }

            } else {

                // News on accepted socket!

                char buffer[READ_BUFFER_SIZE];

                ssize_t r;

                while ((r = read(pdata->fd, buffer, READ_BUFFER_SIZE)) > 0) {

                    printf("SV %s %d CL %s %d: received \"%.*s\".\n", sv_ntop, sv_port, pdata->ntop, pdata->port, (int) r, buffer);

                }

                if ((r == 0) || ((r == -1) && (errno == ECONNRESET))) {

                    del_socket_from_epoll(epollfd, pdata->fd);
                    close(pdata->fd);

                    printf("SV %s %d CL %s %d: connection closed.\n", sv_ntop, sv_port, pdata->ntop, pdata->port);

                    free(pdata);

                } else if ((r == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {

                    // Socket is OK: all data is read so far...

                } else {

                    printf("SV %s %d CL %s %d: read error (errno = %d, strerror = \"%s\").\n", sv_ntop, sv_port, pdata->ntop, pdata->port, errno, strerror(errno));

                }

            }

        }

    }

    // TODO: free memory allocated for struct epoll_event_data...

    // Closing epoll, closing listening socket...

    close(epollfd);
    close(listsockfd);

    printf("SV %s %d: closed epoll, closed epoll.\n", sv_ntop, sv_port);

    return EXIT_SUCCESS;

}
