#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <arpa/inet.h>

struct addrinfo hints, *result;

int
create_socket(struct addrinfo *p)
{  
    char host[256];
    int sock_fd;

    if (getnameinfo(p->ai_addr,
                p->ai_addr->sa_len,
                host, 256,
                NULL, 0, 0)) {
        perror("getnameinfo()");
        exit(1);
    }
    printf("host name: %s\n", host);

    if ((sock_fd = socket(p->ai_family, p->ai_socktype, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if (bind(sock_fd, p->ai_addr, p->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }
    if (getsockname(sock_fd, result->ai_addr, &result->ai_addrlen)) {
        perror("getting socket name");
        exit(EXIT_FAILURE);
    }
    if (listen(sock_fd, 128) != 0) {
        perror("listen()");
        exit(1);
    }
    struct sockaddr_in *result_addr =  (struct sockaddr_in *)result->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));
    printf("Waiting for connection...\n");
    return sock_fd;
}

void
handle_socket(int sock_fd)
{
    int fd, rval;
	char claddr[INET6_ADDRSTRLEN];
	struct sockaddr_in6 client;
	socklen_t length;

	length = sizeof(client);
	memset(&client, 0, length);

	if ((fd = accept(sock_fd, (struct sockaddr *)&client, &length)) < 0) {
		perror("accept");
		return;
	}

	do {
		char buf[BUFSIZ];
		bzero(buf, sizeof(buf));
		if ((rval = read(fd, buf, BUFSIZ)) < 0) {
			perror("reading stream message");
		}

		if (rval == 0) {
			// (void)printf("Ending connection\n");
		} else {
			const char *rip;
			if ((rip = inet_ntop(PF_INET6, &(client.sin6_addr), claddr, INET6_ADDRSTRLEN)) == NULL) {
				perror("inet_ntop");
				rip = "unknown";
			} else {
				(void)printf("Client (%s) sent: %s\n", rip, buf);
			}
		}
	} while (rval != 0);
	(void)close(fd);
}

int
main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: host\n");
        exit(1);
    }

    struct addrinfo *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int s = getaddrinfo(argv[1], "8080", &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    
    int n_socks;
    for (p = result, n_socks = 0; p != NULL; p = p->ai_next, ++n_socks);
    
    int sock_fds[n_socks];
    
    for (p = result, n_socks = 0; p != NULL; p = p->ai_next, ++n_socks) {
        sock_fds[n_socks] = create_socket(p);
    }
   
    int num_ready, i;
    while (1) {
        fd_set readfds;
        struct timeval timeout;
        
        FD_ZERO(&readfds);
        
        for (i = 0; i < n_socks; i++) {
            FD_SET(sock_fds[i], &readfds);
        }

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        num_ready = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
        
        if (num_ready < 0) {
            perror("error in select()");
        } else if (num_ready == 0) {
            // printf("timeout\n");
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
