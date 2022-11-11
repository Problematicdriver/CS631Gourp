#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

struct addrinfo hints, *infoptr;

int
main(int argc, char **argv)
{
    if (argc != 2) {
        perror("argc");
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    int result = getaddrinfo(
            argv[1],
            NULL, &hints, &infoptr);
    if (result) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        exit(1);
    } 
    struct addrinfo *p;
    char host[256], server[256];
    
    for (p = infoptr; p != NULL; p = p->ai_next) {
        getnameinfo(
                p->ai_addr,
                p->ai_addrlen,
                host, sizeof(host), server, 256,
                NI_NUMERICHOST);
        puts(host);
        // puts(server);
    }
}
