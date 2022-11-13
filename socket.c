#include "socket.h"
#define BACKLOG 128

struct addrinfo hints, *result;

char* hostname;
char* port;
bool d_FLAG;

int
allocate_fd(struct addrinfo *p)
{
    char host[256];
    int sock_fd;

    if (getnameinfo(p->ai_addr,
                p->ai_addr->sa_len,
                host, 256,
                NULL, 0, 0)) {
        if (d_FLAG) fprintf(stderr, "getnameinfo()");
        return -1;
    }
    printf("host found: %s\n", host);

    if ((sock_fd = socket(p->ai_family, p->ai_socktype, 0)) < 0) {
        if (d_FLAG) fprintf(stderr, "socket()");
        return -1;
    }

    if (bind(sock_fd, p->ai_addr, p->ai_addrlen) != 0) {
        if (d_FLAG) fprintf(stderr, "bind()");
        return -1;
    }
    if (getsockname(sock_fd, p->ai_addr, &p->ai_addrlen)) {
        if (d_FLAG) fprintf(stderr, "getsockname()");
        return -1;
    }
    if (listen(sock_fd, 128) != 0) {
        if (d_FLAG) fprintf(stderr, "listen()");
        return -1;
    }
    struct sockaddr_in *result_addr =  (struct sockaddr_in *)p->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));
    printf("Waiting for connection...\n");
    return sock_fd;
}

int
create_socket()
{
    int sock_fd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;    
    hints.ai_socktype = SOCK_STREAM;

    // char host[256], server[256];
    
    printf("looking for %s, port %s ...\n", hostname, port);
    
    int s = getaddrinfo(hostname, port, &hints, &result);
    
    if (s != 0) {
        if (d_FLAG) {
            (void)fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        }
        return 1;
    }
   
    if ((sock_fd = allocate_fd(result)) < 0) {
        fprintf(stderr, "allocate_fd");
        return 1;
    }

    /* Loop for listening client socket connection*/
    char buffer[1024];
    for (;;) {
        struct sockaddr_in cliAddr;
        socklen_t cliAddr_size;
        /* Recieve client request */
        int client_fd = accept(sock_fd, (struct sockaddr*)&cliAddr, &cliAddr_size);
        if (client_fd < 0) {
            printf("One Request Passed!");
            continue;
        }
        printf("Connection accepted from %s:%d\n\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

        /* Fork one process for one client request */
        int childpid;
        if ((childpid = fork()) == 0) {
            close(sock_fd);
            /* Handle the request/Send the server response */
            char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
            for (unsigned long sent = 0; sent < sizeof(response); sent += send(client_fd, response+sent, sizeof(response)-sent, 0));
            
            recv(client_fd, buffer, 1024, 0);
            printf("PID[%d] body:\n\n%s\n",getpid(), buffer);
            /* Close the client socket connection */
            close(client_fd);
        }
    }
    return 0;
}
