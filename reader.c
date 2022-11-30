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
            char response[] = "HTTP/1.0 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
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
        (strncmp(protocol, "HTTP/0.9", 9) == 0));
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

int
isValidDate(char *date, struct tm *tm) {
    /* this part needs work */
    return 0;
}

int isValidContent(char *content)
{
    return (strcmp(content, "Date") == 0 ||
        strcmp(content, "Server") == 0 ||
        strcmp(content, "Last-Modified") == 0 ||
        strcmp(content, "Content-Type") == 0 ||
        strcmp(content, "Content-Length") == 0);
}

int
isValidHeader(char *line, int *checkDate, struct tm *tm) 
{
    /* Content-Type: text/html; charset=utf-8 */

    char dup_line = strdup(line);
    char *tok, *const d = "If-Modified-Since";

    if (dup_line[0] == ':') {
        /* should not have start with ':' */
        fprintf(stderr, "invalid header");
        return 1;
    }

    tok = strtok(dup_line, ":");
    if (isValidContent(tok)) {
        fprintf(stderr, "invalid header");
        return 1;
    }
    if (strcmp(tok, d) == 0) {
        /* if this is Modified-Since */
        if (checkDate == 1) {
            /* duplicate Modified-Since */
            fprintf(stderr, "dulplicate If-Modified-Since");
            return 1;
        }
        *checkDate = 1;
    }

    tok = strtok(NULL, "");
    if (tok == NULL) {
        /* should be exactly 2 tokens */
        fprintf(stderr, "invalid header");
        return 1;
    }
    if (*checkDate == 1) {
        /* this token is a date string */
        if (isValidDate(tok, tm) != 0) {
            return 1;
        }
    }
    return 0;
}

void
updatePath(char** updated_path, char* path, char* initial) {
    char* part;
    int index;
    int size;
    part = strtok(strdup(path), "/");
    index = 0;
    size = 0;

    while (part != NULL) {
        if(index == 0) {
            size = strlcat((char *)*updated_path, initial, PATH_MAX - strlen((const char *)*updated_path));
            ((char *)*updated_path)[size + 1] = '\0';
        } else {
            if (strncmp(part, "..", 3) == 0) {
                char* update, *previous;
                char* tmp = strdup((char *)*updated_path);
                while ((update = strstr(strdup(tmp), "/")) != NULL) {
                    tmp = update + 1;
                    previous = strdup(tmp);
                }
                if ((update = strstr((char *)*updated_path, previous)) != NULL) {
                    int current_index = 0;
                    current_index = update - (char *)*updated_path;
                    ((char *)*updated_path)[current_index - 1] = '\0';
                }
            } else {
                size = strlcat((char *)*updated_path, "/", PATH_MAX - strlen((const char *)*updated_path));
                ((char *)*updated_path)[size + 1] = '\0';
                size = strlcat((char *)*updated_path, part, PATH_MAX - strlen((const char *)*updated_path));
                ((char *)*updated_path)[size + 1] = '\0';
            }
        }
        index++;
        part = strtok(NULL, "/");
    }
}

char *
checkPath(char* path) {
    char* newpath;
    char* host;
    char* part;
    char* updated_path;
    int size;

    if (path[0] != '/') {
        /* Remove schema and hostname from path. */
        if ((host = strstr(strdup(path), "://")) != NULL) {
            host = (host+3);
        } else {
            host = strdup(path);
        }
        newpath = strstr(strdup(host),"/");
    } else {
        newpath = strdup(path);
    }


    if ((updated_path = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
        exit(1);
    } 
    updated_path[0] = '\0';

    if (isPrefix(newpath, "/cgi-bin/") && c_FLAG) {
        /* 
         * /cgi-bin/ can be a valid directory within the docroot. 
         * Only convert /cgi-bin/ into the path to cgidir if the c flag is set.
         */
        updatePath(&updated_path, newpath, real_cgidir);
    } else if ((!isPrefix(newpath, "/~/") && isPrefix(newpath, "/~"))) {
    
        /* Resolve ~user to home directory of that user. */
        char *homedir;
        struct passwd *user;

        if ((homedir = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
            if (d_FLAG) {
                (void)printf("malloc: %s\n", strerror(errno));
            }
            exit(1);
        } 

        part = strtok(strdup(newpath), "/");
        if ((user = getpwnam(part+1)) == NULL) {
            /* error here */
            fprintf(stderr,"err: %s\n", strerror(errno));
            exit(1);
        }
    
        size = strlcat(homedir, user->pw_dir, PATH_MAX - strlen(homedir));
        homedir[size + 1] = '\0';
        size = strlcat(homedir, "/sws", PATH_MAX - strlen(homedir));
        homedir[size + 1] = '\0';
        updatePath(&updated_path, newpath, homedir);
    } else {
        /* No special case, prepend real_docroot. */
        updatePath(&updated_path, newpath, real_docroot);
    }
    
    char* real_updated;
    if ((real_updated = (char *)malloc(sizeof(char)*PATH_MAX)) == NULL) {
        (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
        exit(1);
    } else if (realpath(updated_path, real_updated) == NULL) {
        (void)fprintf(stderr, "realpath of %s: %s\n", updated_path, strerror(errno));
        exit(1);
    } else {
        if (d_FLAG) {
            (void)printf("The realpath of docroot %s is %s\n", updated_path, real_updated);
        }
    }
    return real_updated;
}

void
logging(char* remoteAddress, char* reqestedTime, char* firstLineOfRequest, char* status, char* responseSize) {
    char* logging_buffer;
    int n;

    if ((logging_buffer = (char *)malloc(sizeof(char)*BUFSIZE)) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
		exit(1);
    } 
    sprintf(logging_buffer, "%s %s %s %s %s", remoteAddress, reqestedTime, firstLineOfRequest, status, responseSize);
    if(n = write(logFD, logging_buffer, sizeof(logging_buffer)) == -1){
        if (d_FLAG) {
            (void)printf("Error while logging into file: %s\n", strerror(errno));
        }
		exit(1);
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
    int checkDate = 0;
    int f_invalid_header;

    struct tm tm;
    memset(&tm, 0, sizeof(tm));

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
                        (void)printf("405 Method Not Allowed");
                    }
                    printf("Mehotd Error\n");
                    return "405 Method Not Allowed";
                } else if (!checkProtocol(protocol)){
                    printf("Protocol Error\n");
                    return "CHANGE";
                }
                path = checkPath(path);
                /* Actually serve. */

                
                /* Logging if l flag is given 
                if(l_FLAG) {
                    logging(method, path, protocol, response);
                }
                */
                
            }
        } else {
            /* (Header) Anything other than First line */
            f_invalid_header = isValidHeader(line, &checkDate, &tm);
        }
        line = strtok(NULL, "\r\n");
        n++;
    }
    return "CHANGE";
}
