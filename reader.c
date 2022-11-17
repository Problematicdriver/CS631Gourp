#include "reader.h"

void handle_socket(int server_fd) {
    /* Buffer for storing client request */
    char *s;
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
        // printf("Connection accepted from %s:%d\n\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
        printf("Connection accepted");
        
        /* Fork one process for one client request */
        if ((childpid = fork()) == 0) {
            close(server_fd);
            /* Send the server response */
            // char response[] = "HTTP/1.0 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
            // for (unsigned long sent = 0; sent < sizeof(response); sent += send(client_fd, response+sent, sizeof(response)-sent, 0));
            /* Handle client request */
            // recv(client_fd, buffer, 1024, 0);

            s = reader(server_fd);
            
            printf("PID[%d] body:\n\n%s\n",getpid(), buffer);
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
    char* real;
    char* docroot;
    docroot = realpath(".");
    real = realpath(path);
    if (strncmp(docroot, real, strlen(docroot)) == 0) {
        return real;
    } else {
        return docroot;
    }
}

char *
reader(int fd) {
    char buf[BUFSIZE];
    char* method;
    char* part;
    char* path;
    char* protocol;
    int index;

    recv(client_fd, buf, BUFSIZE, 0);

    char* line = NULL;
    line = strtok(buf, "\r\n");

    printf("Request:\n");
    bool firstLine = true;
    while (line != NULL) {
        printf("[%d]%s\n",n, line);
        if (firstLine) {
            /* First line */
            printf("[%d]%s\n",n, line);
            // char *method, *part, *protocol
            part = strtok(line, " ");
            index = 0;

            while (part != NULL) {
                if (index == 0) {
                    method = strdup(part);
                } else if (index == 1) {
                    part = strdup(part);
                } else if (index == 2) {
                    protocol = strdup(part);
                }
                index++;
                printf("[%d]%s\n",index, part);
                part = strtok(NULL, " ");
            }
            
            if (index != 2) {
                /* Error here */
            } else {
                if (!checkMethod(method)) {
                    /* Method was not "GET" or "HEAD" */
                    if (d_flag) {
                        fprintf(stderr, "405 Method Not Allowed");
                    }
                    return "405 Method Not Allowed";
                } else if (!checkProtocol(protocol)){
                    return;
                } else if (!checkPath(path)) {
                    return;
                } else {
                    /* Everything is good for frist line */
                }
            }
        } else {
            /* (Header) Anything other than First line */

        }
        line = strtok(NULL, "\r\n");
        firstLine = false;
    }
}
