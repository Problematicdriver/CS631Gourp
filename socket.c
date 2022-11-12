#include "socket.h"
#define BACKLOG 128

struct addrinfo hints, *result;

char* _hostname;
char* _port;
bool d_FLAG;

int
create_socket()
{
    int sock_fd;

    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_socktype = SOCK_STREAM;

    char *hostname, *port, host[256];
    hostname = _hostname;
    port = _port;
    int s = getaddrinfo(hostname, port, &hints, &result);
    if (s != 0) {
        if (d_FLAG) {
            (void)fprintf(stdout, "getaddrinfo: %s\n", gai_strerror(s));
        }
        return 1;
    }
    if ((sock_fd = socket(AF_INET6, SOCK_STREAM, 0) < 0)) {
        if (d_FLAG) {
            (void)fprintf(stdout, "socket()");
        }
        return 2;
    }
    if (getnameinfo(result->ai_addr, result->ai_addr->sa_len, host, 256, NULL, 0, 0)) {
        (void)fprintf(stdout, "getnameinfo()");
        return 3;
    }
    printf("host found: %s\n", host);
    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
        (void)fprintf(stdout, "bind()");
        return 5;
    }
    if (listen(sock_fd, BACKLOG) != 0) {
        (void)fprintf(stdout, "listen()");
        return 6;
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