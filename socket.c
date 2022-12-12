#include "socket.h"
#include "reader.h"

#define TIMEOUT 10
#define BACKLOG 512
#define DBG_BACKLOG 1

struct addrinfo hints, *result, *p;

int
allocate_fd(struct addrinfo *p) {
    char host[256];
    int sock_fd;

    if (getnameinfo(p->ai_addr,
                p->ai_addr->sa_len,
                host, 256,
                NULL, 0, 0)) {
        if (d_FLAG) {
            (void)printf("getnameinfo()\n");
        }
        return -1;
    }
    if (d_FLAG) {
        (void)printf("host found: %s\n", host);    
    }
    
    if ((sock_fd = socket(p->ai_family, p->ai_socktype, 0)) < 0) {
        if (d_FLAG) {
            (void)printf("socket()\n");
        }
        return -1;
    }

    if (bind(sock_fd, p->ai_addr, p->ai_addrlen) != 0) {
        if (d_FLAG) {
            (void)printf("bind(), %s\n", strerror(errno));
        }
        return -1;
    }
    if (getsockname(sock_fd, p->ai_addr, &p->ai_addrlen)) {
        if (d_FLAG) {
            (void)printf("getsockname()\n");
        }
        return -1;
    }
    if (listen(sock_fd, d_FLAG ? DBG_BACKLOG : BACKLOG) != 0) {
        if (d_FLAG) {
            (void)printf("listen()\n");
        }
        return -1;
    }
    struct sockaddr_in *result_addr =  (struct sockaddr_in *)p->ai_addr;
    if (d_FLAG) {
        (void)printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));
        (void)printf("Waiting for connection...\n");
    }
    return sock_fd;
}

int
socket_select() {   
    int n_socks, num_ready, i;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;    
    hints.ai_socktype = SOCK_STREAM;
    
    if (d_FLAG) {
        (void)printf("looking for %s, port %s ...\n", hostname, port);
    }
    int s = getaddrinfo(hostname, port, &hints, &result);
    
    if (s != 0) {
        if (d_FLAG) {
            (void)printf("getaddrinfo: %s\n", gai_strerror(s));
        }
        return 1;
    }
   
    for (p = result, n_socks = 0; p != NULL; p = p->ai_next, ++n_socks);
    int sock_fds[n_socks];

    for (p = result, n_socks = 0; p != NULL; p = p->ai_next, ++n_socks) {
        if ((sock_fds[n_socks] = allocate_fd(p)) < 0) {
            if (d_FLAG) {
                (void)printf("allocate_fd");
            }
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
            if (d_FLAG) {
                (void)printf("select");    
            }
            return 1;
        } else {
            for (i = 0; i < n_socks; i++) {
                if (FD_ISSET(sock_fds[i], &readfds)) {
                    handle_socket(sock_fds[i]);
                }
            }
        }
    }
    return 0;
}
