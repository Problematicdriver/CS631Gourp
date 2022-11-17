#include "reader.h"

void handle_socket(int server_fd) {
    /* Buffer for storing client request */
    int client_fd, childpid;
    for (;;) {
        // struct sockaddr_in cliAddr;
        // socklen_t cliAddr_size;
        
        /* Recieve client request */
        // int client_fd = accept(server_fd, (struct sockaddr*)&cliAddr, &cliAddr_size);
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            continue;
        }
        //printf("Connection accepted from %s:%d\n\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
        //printf("Connection accepted");
        
        /* Fork one process for one client request */
        if ((childpid = fork()) == 0) {
            close(server_fd);
            char *s;
            // Reader
            s = reader(client_fd);
            printf("[reader return]%s\n\n", s);

            // Writer
            char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
            for (unsigned long sent = 0; sent < sizeof(response); sent += send(client_fd, response+sent, sizeof(response)-sent, 0));

            /* Close the client socket connection */
            close(client_fd);
            exit(1);
        } else if (childpid < 0) {
            perror("fork()");
        } else {
            wait(NULL);
        }
    }
}

bool checkProtocol(char* protocol) {
    return ((strncmp(protocol, "HTTP/1.0", 9) == 0) ||
        (strncmp(protocol, "HTTP/0.9", 9) == 0));
}

bool checkMethod(char* method) {
    return ((strncmp(method, "GET", 4) == 0) ||
        (strncmp(method, "HEAD", 5) == 0));
}

char* checkPath(char* path) {
    // char* real;
    // char* docroot;
    // docroot = realpath(".");
    // real = realpath(path);
    // if (strncmp(docroot, real, strlen(docroot)) == 0) {
    //     return real;
    // } else {
    //     return docroot;
    // }
    return path;
}

char *
reader(int fd) {
    char buf[BUFSIZE];
    char* method;
    char* part;
    char* path;
    char* protocol;
    int index;

    recv(fd, buf, BUFSIZE, 0);
    int n = 0;
    char* lines[1024] = {NULL};
    char* line = NULL;
    line = strtok(buf, "\r\n");
    /* Read every line into *lines[1024] */
    while (line != NULL) {
        lines[n] = strdup(line);
        n++;
        line = strtok(NULL, "\r\n");
    }
    n = 0;
    while (lines[n] != NULL) {
        line = lines[n];
        printf("[%d]%s\n",n+1, line);
        if (n == 0) {
            /* First line */
            printf("[First Line]\n");
            // char *method, *part, *protocol
            part = strtok(line, " ");
            index = 0;

            while (part != NULL) {
                if (index == 0) {
                    method = strdup(part);
                } else if (index == 1) {
                    path = strdup(part);
                } else if (index == 2) {
                    protocol = strdup(part);
                }
                index++;
                printf("\t[%d]%s\n",index, part);
                part = strtok(NULL, " ");
            }
            
            if (index != 2) {
                /* Error here */
            } else {
                if (!checkMethod(method)) {
                    /* Method was not "GET" or "HEAD" */
                    if (d_FLAG) {
                        fprintf(stderr, "405 Method Not Allowed");
                    }
                    return "405 Method Not Allowed";
                } else if (!checkProtocol(protocol)){
                    return "CHANGE";
                } else if (!checkPath(path)) {
                    return "CHANGE";
                } else {
                    /* Everything is good for frist line */
                }
            }
        } else {
            /* (Header) Anything other than First line */
        }
        line = strtok(NULL, "\r\n");
        n++;
    }
    return "CHANGE";
}
