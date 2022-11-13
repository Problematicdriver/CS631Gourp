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

int
main(int argc, char **argv)
{
    if (argc < 2) {
        perror("Usage: ...");
        exit(1);
    }
    
    /* establish connection */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
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
        (void)printf("connection to %s successful\n", host);
    }
    /* by now socketfd should have been connected */
    freeaddrinfo(p0);
}
