#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>

struct addrinfo hints, *infoptr;

int
main(int argc, char **argv)
{
    if (argc != 2) {
        perror("argc");
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;

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
                host, sizeof(host), server, sizeof(server),
                NI_NUMERICHOST);
        puts(host);
        struct sockaddr_in *result_addr =  (struct sockaddr_in *)p->ai_addr;
        // printf("port %d\n", ntohs(result_addr->sin_port));
    }
}
