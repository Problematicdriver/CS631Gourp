#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct _host_info {
    char *hostname;
    char *port;
    char *resource;
} host_info;

struct addrinfo hints, *p0;

void send_request(host_info *info) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Re-use address is a little overkill here because we are making a
    // Listen only server and we don’t expect spoofed requests.
    int optval = 1;
    int retval = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
    sizeof(optval));
    if(retval == -1) {
        perror("setsockopt");
        exit(1);
    }
    // Connect using code snippet
    struct addrinfo current, *result;
    memset(&current, 0, sizeof(struct addrinfo));
    current.ai_family = AF_INET;
    current.ai_socktype = SOCK_STREAM;
    getaddrinfo(info->hostname, info->port, &current, &result);
    connect(sock_fd, result->ai_addr, result->ai_addrlen);

    // Send the get request
    // Open so you can use getline
    FILE *sock_file = fdopen(sock_fd, "r+");
    setvbuf(sock_file, NULL, _IONBF, 0);
    ret = handle_okay(sock_file);
    fclose(sock_file);
    close(sock_fd);
}

int
main(int argc, char **argv)
{
    if (argc != 3) {
        perror("Usage: ...");
    }
    
    /* establish connection */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    
    int result;
    result = getaddrinfo(argv[1], argv[2], &hints, &p0);
    
    if (result) {
        const char *mesg = gai_strerror(result);
        fprintf(stderr, "getaddrinfo: %s\n", mesg);
        exit(1);
    }
    
    printf("addresses found for %s\n", argv[1]);

    int socketfd;
    struct addrinfo *p = p0;
    char host[256];
    for (p = p0; p != NULL; p = p->ai_next) {
        socketfd = socket(p->ai_family, p->ai_socktype, 0);
        if (socketfd < 0) {
            perror("socket()");
            continue;
        }
        (void)getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host),
                NULL, 0, NI_NUMERICHOST);
        (void)printf("connecting to %s ...\n", host);
        if (connect(socketfd, p->ai_addr, p->ai_addrlen) < 0) {
            (void)printf("connection to %s failed\n", host);
            close(socketfd);
            continue;
        }
    }
    /* by now socketfd should have been connected */
    freeaddrinfo(p0);
}
