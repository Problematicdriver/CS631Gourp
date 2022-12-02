#include "socket.h"
#include "reader.h"

#define TIMEOUT 3
#define BACKLOG 128
#define DBG_BACKLOG 1

bool c_FLAG;
bool d_FLAG;
bool h_FLAG;
bool i_FLAG;
bool l_FLAG;
bool p_FLAG;

char *cgidir;
char *docroot;
char *hostname;
char *port;
char *real_cgidir;
char *real_docroot;
int logFD;

struct addrinfo hints, *result, *p;

int
allocate_fd(struct addrinfo *p)
{
    char host[256];
    int sock_fd;

    if (getnameinfo(p->ai_addr,
                p->ai_addr->sa_len,
                host, 256,
                NULL, 0, 0)) {
        if (d_FLAG) fprintf(stderr, "getnameinfo()\n");
        return -1;
    }
    printf("host found: %s\n", host);

    if ((sock_fd = socket(p->ai_family, p->ai_socktype, 0)) < 0) {
        if (d_FLAG) fprintf(stderr, "socket()\n");
        return -1;
    }

    if (bind(sock_fd, p->ai_addr, p->ai_addrlen) != 0) {
        if (d_FLAG) fprintf(stderr, "bind(), %s\n", strerror(errno));
        return -1;
    }
    if (getsockname(sock_fd, p->ai_addr, &p->ai_addrlen)) {
        if (d_FLAG) fprintf(stderr, "getsockname()\n");
        return -1;
    }
    if (listen(sock_fd, d_FLAG ? DBG_BACKLOG : BACKLOG) != 0) {
        if (d_FLAG) fprintf(stderr, "listen()\n");
        return -1;
    }
    struct sockaddr_in *result_addr =  (struct sockaddr_in *)p->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));
    printf("Waiting for connection...\n");
    return sock_fd;
}

int
socket_select()
{   
    int n_socks, num_ready, i;

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
   
    for (p = result, n_socks = 0; p != NULL; p = p->ai_next, ++n_socks);
    int sock_fds[n_socks];

    for (p = result, n_socks = 0; p != NULL; p = p->ai_next, ++n_socks) {
        if ((sock_fds[n_socks] = allocate_fd(p)) < 0) {
            fprintf(stderr, "allocate_fd");
            return 1;
        }
    }

    while (1) {
        fd_set readfds;
        struct timeval timeout; 
        
        FD_ZERO(&readfds);
        
        for (i = 0; i < n_socks; i++) {
            FD_SET(sock_fds[i], &readfds);
        }
        
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec =  0;

        num_ready = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);

        if (num_ready < 0) {
            fprintf(stderr, "allocate_fd");
            return 1;
        } else if (num_ready == 0) {
            /* Time out */
        } else {
            for (i = 0; i < n_socks; i++) {
                if (FD_ISSET(sock_fds[i], &readfds)) {
                    /* reader function being called here instead of sleep */
                    handle_socket(sock_fds[i]);
                }
            }
        }
    }

    return 0;
    
    // /* Loop for listening client socket connection*/
    // char buffer[1024];
    // for (;;) {
    //     struct sockaddr_in cliAddr;
    //     socklen_t cliAddr_size;
    //     /* Recieve client request */
    //     int client_fd = accept(sock_fd, (struct sockaddr*)&cliAddr, &cliAddr_size);
    //     if (client_fd < 0) {
    //         printf("One Request Passed!");
    //         continue;
    //     }
    //     printf("Connection accepted from %s:%d\n\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

    //     /* Fork one process for one client request */
    //     int childpid;
    //     if ((childpid = fork()) == 0) {
    //         close(sock_fd);
    //         /* Handle the request/Send the server response */
    //         char response[] = "HTTP/1.0 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
    //         for (unsigned long sent = 0; sent < sizeof(response); sent += send(client_fd, response+sent, sizeof(response)-sent, 0));
            
    //         recv(client_fd, buffer, 1024, 0);
    //         printf("PID[%d] body:\n\n%s\n",getpid(), buffer);
    //         /* Close the client socket connection */
    //         close(client_fd);
    //     }
    // }
    // return 0;
}
