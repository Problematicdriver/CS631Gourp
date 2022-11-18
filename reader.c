#include "reader.h"

void 
handle_socket(int server_fd) {
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

            // Writer: Return a Hello world just for showcase
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

bool 
checkProtocol(char* protocol) {
    return ((strncmp(protocol, "HTTP/1.0", 9) == 0) ||
        (strncmp(protocol, "HTTP/0.9", 9) == 0) || 
        (strncmp(protocol, "HTTP/1.1", 9) == 0));;
}

bool 
checkMethod(char* method) {
    return ((strncmp(method, "GET", 4) == 0) ||
        (strncmp(method, "HEAD", 5) == 0));
}

bool
isPrefix(char* string, char* prefix) {
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

char *
checkPath(char* path) {

    char* part;
    char* updated_path = (char *)malloc(sizeof(char)*PATH_MAX);

    if(updated_path == NULL){
        (void)printf("Error while allocating buffer : %s\n", strerror(errno));
		exit(1);
    }
    updated_path[0] = '\0';

    if(isPrefix(path, "/cgi-bin")) {

        if(!c_FLAG) {
            (void)printf("Error c flag is not found : %s\n", strerror(errno));
            exit(1);
        }

        int index;
        part = strtok(path, "/");
        index = 0;

        while (part != NULL) {
            if (index == 0) {
                if (strcat(updated_path, cgiDir) == NULL){
                     (void)printf("Error while processing path : %s\n", strerror(errno));
                     exit(1);
                }
            } else {
                if (strcat(updated_path, "/") == NULL){
                     (void)printf("Error while processing path : %s\n", strerror(errno));
                     exit(1);
                }
                if (strcat(updated_path, part) == NULL){
                     (void)printf("Error while processing path : %s\n", strerror(errno));
                     exit(1);
                }
            }
            index++;
            part = strtok(NULL, "/");
        }
    } else {
        part = strtok(path, "?");
        if((updated_path = strdup(part))==NULL) {
            (void)printf("Error while processing path : %s\n", strerror(errno));
            exit(1);
        }
    }
    return updated_path;
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
            // First line should be 3 fields, or it's invalid
            // method path protocol
            if (index != 3) {
                /* Error here */
            } else {
                if (!checkMethod(method)) {
                    /* Method was not "GET" or "HEAD" */
                    if (d_FLAG) {
                        fprintf(stderr, "405 Method Not Allowed");
                    }
                    printf("Mehotd Error\n");
                    return "405 Method Not Allowed";
                } else if (!checkProtocol(protocol)){
                    printf("Protocol Error\n");
                    return "CHANGE";
                } else if ((path = checkPath(path)) == NULL) {
                    return "CHANGE";
                } else {
                    /* Everything is good for first line */
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
