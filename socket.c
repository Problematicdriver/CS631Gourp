#include "socket.h"
#include "reader.h"

#define TIMEOUT 10
#define BACKLOG 128
#define DBG_BACKLOG 1

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
            (void)fprintf(stderr, "allocate_fd");
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
}
