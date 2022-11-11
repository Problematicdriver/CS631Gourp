#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <unistd.h>

int
main()
{
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    char host[256];

    int s = getaddrinfo(NULL, "1234", &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    int sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
    
    if (getnameinfo(result->ai_addr,
                result->ai_addr->sa_len,
                host, 256,
                NULL, 0, 0)) {
        perror("getnameinfo()");
        exit(1);
    }
    printf("host name: %s\n", host);

    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }
    if (listen(sock_fd, 10) != 0) {
        perror("listen()");
        exit(1);
    }
    struct sockaddr_in *result_addr =  (struct sockaddr_in *)result->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));
    printf("Waiting for connection...\n");
    
    int client_fd = accept(sock_fd, NULL, NULL);
    printf("Connection made: client_fd=%d\n", client_fd);

    close(sock_fd);
}
