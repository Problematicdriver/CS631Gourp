#include "reader.h"

static const char *rfc1123_date = "%a, %d %B %Y %T %Z";
static const char *rfc850_date = "%a, %d-%B-%y %T %Z";
static const char *ansic_date = "%a %B %d %T %Y";
static const char *request_date_format = "%FT%TZ";

char valid_status_codes[][FIELD_SIZE] = {
    "200",
    "400",
    "403",
    "405",
    "415",
    "431",
    "500"
};

void 
handle_socket(int server_fd) {
    /* Buffer for storing client request */
    int client_fd, childpid;
    for (;;) {
        struct sockaddr_in cliAddr;
        socklen_t cliAddr_size;
        
        /* Recieve client request */
        client_fd = accept(server_fd, (struct sockaddr*)&cliAddr, &cliAddr_size);
        // client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            continue;
        }

        char* remoteIp = (char *)malloc(sizeof(char)*BUFSIZ);
        sprintf(remoteIp, "%s:%d", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
        //printf("Connection accepted");
        
        /* Fork one process for one client request */
        if ((childpid = fork()) == 0) {
            close(server_fd);
            //Find the time and store

            time_t currentTime;
            char requestTime[100]; 
            struct tm *tm;                                                                                                                       
            currentTime = time(NULL);                                                              
            tm = localtime(&currentTime);      
            strftime(requestTime, sizeof(requestTime), request_date_format, tm);

            reader_response r_response;
            // Reader
            r_response = reader(client_fd);
            r_response.requestTime = requestTime;
            r_response.remoteIp = remoteIp;
            // Writer: Return a Hello world just for showcase
            writer(r_response, client_fd);
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

static time_t mtime;

int
isValidDate(char *date) {
    struct tm tm;
    int ret;
    char *s;
    (void)memset(&tm, 0, sizeof(tm));
    
    if ((s = strptime(date, rfc1123_date, &tm)) != NULL)
        ret = 1;
    else if ((s = strptime(date, rfc850_date, &tm)) != NULL)
        ret = 1;
    else if ((s = strptime(date, ansic_date, &tm)) != NULL)
        ret = 1;    
    if ((mtime = mktime(&tm)) == -1) {
        if (d_FLAG) {
            (void)printf("mktime() %s\n", strerror(errno));
        }
        ret = 0;
    }
    return ret;
}

int
getHeaderContent(char *line) {
    char header[FIELD_SIZE], field[FIELD_SIZE];
    
    if (sscanf(line," %[^: ] : %[^\t]", header, field) < 2) {
        return 400;
    }

    if ((strlen(header) > FIELD_SIZE) || (strlen(field) > FIELD_SIZE)) {
        return 431;
    }

    if (strncmp(header, "If-Modified-Since", 18) == 0) {
        if (isValidDate(field) != 1) {
            if (d_FLAG){
                (void)printf("Invalid time format in header: %s\n", line);
            }
            return 400;
        }
    }
    
    return 0;
}

void
updatePath(char** updated_path, char* path, char* initial) {
    char* part;
    int size;
    part = strtok(strdup(path), "/");
    size = strlcat((char *)*updated_path, initial, PATH_MAX - strlen((const char *)*updated_path));
    ((char *)*updated_path)[size + 1] = '\0';

    while (part != NULL) {
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
        part = strtok(NULL, "/");
    }
}

char *
checkPath(char* path) {
    char* newpath;
    char* host;
    char* part;
    char* prefix_root;
    char* updated_path;
    int size;

    if ((prefix_root = strdup(real_docroot)) == NULL) {
        if (d_FLAG) {
            (void)printf("strdup: %s\n", strerror(errno));
        }
        return "500 Internal Server Error";
    }

    if (path[0] != '/') {
        /* Remove schema and hostname from path. */

        // TODO add strdup check but really strdup might not need to be here
        if ((host = strstr(strdup(path), "://")) != NULL) {
            host = (host+3);
        } else {
        // TODO add strdup check but really strdup might not need to be here
            host = strdup(path);
        }
        // TODO add strdup check but really strdup might not need to be here
        newpath = strstr(strdup(host), "/");
        free(host);
    } else {
        if ((newpath = strdup(path)) == NULL) {
            if (d_FLAG) {
                (void)printf("strdup: %s\n", strerror(errno));
            }
            free(prefix_root);
            return "500 Internal Server Error";
        }
    }

    if ((updated_path = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
        free(prefix_root);
        free(newpath);
        return "500 Internal Server Error";
    } 
    updated_path[0] = '\0';

    if (isPrefix(newpath, "/cgi-bin/") && c_FLAG) {
        /* 
         * /cgi-bin/ can be a valid directory within the docroot. 
         * Only convert /cgi-bin/ into the path to cgidir if the c flag is set.
         */
        free(prefix_root);
        if ((prefix_root = strdup(real_cgidir)) == NULL) {
            if (d_FLAG) {
                (void)printf("strdup: %s\n", strerror(errno));
            }
            free(newpath);
            free(updated_path);
            return "500 Internal Server Error";
        }
        updatePath(&updated_path, newpath+8, real_cgidir);
    } else if ((!isPrefix(newpath, "/~/") && isPrefix(newpath, "/~"))) {
        /* Resolve ~user to home directory of that user. */
        char *homedir;
        struct passwd *user;

        if ((homedir = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
            if (d_FLAG) {
                (void)printf("malloc: %s\n", strerror(errno));
            }
            free(prefix_root);
            free(newpath);
            free(updated_path);
            return "500 Internal Server Error";
        } 

        part = strtok(strdup(newpath), "/");
        if ((user = getpwnam(part+1)) == NULL) {
            if (d_FLAG) {
                (void)printf("getpwnam: %s\n", strerror(errno));
            }
            free(prefix_root);
            free(newpath);
            free(homedir);
            free(updated_path);
            return "500 Internal Server Error";
        }
    
        size = strlcat(homedir, user->pw_dir, PATH_MAX - strlen(homedir));
        homedir[size + 1] = '\0';
        size = strlcat(homedir, "/sws", PATH_MAX - strlen(homedir));
        homedir[size + 1] = '\0';

        free(prefix_root);
        if ((prefix_root = strdup(homedir)) == NULL) {
            if (d_FLAG) {
                (void)printf("strdup: %s\n", strerror(errno));
            }
            free(newpath);
            free(homedir);
            free(updated_path);
            return "500 Internal Server Error";
        }
        updatePath(&updated_path, newpath+(strlen(part)+1), homedir);
        free(homedir);
    } else {
        /* No special case, prepend real_docroot. */
        updatePath(&updated_path, newpath, real_docroot);
    }

    if(!isPrefix(updated_path, prefix_root)) {
        /* Do until prefix match and then add all remaining */
        char* part_docroot;
        char* part_updated_path;
        char *p1, *p2;

        // TODO strdup check
        part_docroot = strtok_r(strdup(prefix_root), "/", &p1);
        part_updated_path = strtok_r(strdup(updated_path),"/", &p2);
        
        while (part_docroot != NULL && part_updated_path != NULL) {
            if (
                (strlen(part_docroot) != strlen(part_updated_path)) || 
                (strncmp(part_docroot, part_updated_path, strlen(part_docroot)) != 0)
            ) {
                char* final_updated_path;
                if ((final_updated_path = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
                    if (d_FLAG) {
                        (void)printf("malloc: %s\n", strerror(errno));
                    }
                    free(prefix_root);
                    free(newpath);
                    free(updated_path);
                    return "500 Internal Server Error";
                } 
                final_updated_path[0] = '\0';
                size = strlcat(final_updated_path, prefix_root, PATH_MAX - strlen(final_updated_path));
                final_updated_path[size + 1] = '\0';
                size = strlcat(final_updated_path, "/", PATH_MAX - strlen(final_updated_path));
                final_updated_path[size + 1] = '\0';
                size = strlcat(final_updated_path, part_updated_path, PATH_MAX - strlen(final_updated_path));
                final_updated_path[size + 1] = '\0';
                
                if ((updated_path = strdup(final_updated_path)) == NULL) {
                    if (d_FLAG) {
                        (void)printf("strdup: %s\n", strerror(errno));
                    }
                    free(prefix_root);
                    free(newpath);
                    free(updated_path);
                    return "500 Internal Server Error";
                }
                break;
            }
            part_docroot = strtok_r(NULL, "/", &p1);
            part_updated_path = strtok_r(NULL, "/", &p2);
        }
    }

    char* real_updated;
    if ((real_updated = (char *)malloc(sizeof(char)*PATH_MAX)) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
        free(prefix_root);
        free(newpath);
        free(updated_path);
        return "500 Internal Server Error";
    } else if (realpath(updated_path, real_updated) == NULL) {
        if (d_FLAG) {
            (void)printf("realpath of %s: %s\n", updated_path, strerror(errno));
        }
        free(prefix_root);
        free(newpath);
        free(updated_path);
        return "404 Not Found";
    } else {
        if(!isPrefix(real_updated, prefix_root)) {
            /* 
             * The client has used a symlink to make it outside the docroot.
             * This is the case since all other error checking accounts for the 
             * client leaving the docroot by using /.. within the URI. Client
             * has accessed a forbidden location.
             */
            free(prefix_root);
            free(newpath);
            free(updated_path);
            free(real_updated);
            return "403 Forbidden";
        }
        if (d_FLAG) {
            (void)printf("The realpath of %s is %s\n", updated_path, real_updated);
        }
    }
    return real_updated;
}

reader_response
reader(int fd) {
    char endChar = 127;
    char msgEnd[2];
    char buf[BUFSIZE];
    char* method;
    char* part;
    char* path;
    char* protocol;
    int headerVal;
    int index;
    reader_response r_response;

    msgEnd[0] = endChar;
    msgEnd[1] = '\0';

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
        if (d_FLAG) {
            (void)printf("[%d]%s\n",n+1, line);
            printf("%ld\n", strlen(line));
        }
        if (n == 0) {
            r_response.firstLine = strdup(line);
            if (d_FLAG) {
                (void)printf("[First Line]\n");
            }
            part = strtok(line, " ");
            index = 0;
            while (part != NULL) {
                if (index == 0) {
                    if ((method = strndup(part, 5)) == NULL) {
                        (void)printf("strdup: %s\n", strerror(errno));
                        free(line);
                    }
                } else if (index == 1) {
                    if ((path = strdup(part)) == NULL) {
                        free(line);
                        free(method);
                        (void)printf("strdup: %s\n", strerror(errno));
                    }
                } else if (index == 2) {
                    if ((protocol = strdup(part)) == NULL) {
                        free(line);
                        free(method);
                        free(path);
                        (void)printf("strdup: %s\n", strerror(errno));
                    }
                }
                index++;
                if (d_FLAG) {
                    printf("\t[%d]%s\n", index, part);
                }
                part = strtok(NULL, " ");
            }

            /* 
             * First line send by the client should be exactly 3 seperate parts.
             * These parts should be in the format "METHOD path protocol"
             */
            if (index != 3) {
                /* Error here */
                free(line);
                if (index >= 1) {
                    free(method);
                }
                if (index >= 2) {
                    free(path);
                }
                if (index >= 3) {
                    free(protocol);
                }
                if (d_FLAG) {
                    (void)printf("400 Bad Request");
                }
                r_response.statusCode = 400;
                r_response.path = "";
                r_response.response = "Bad Request";
                return r_response;
            } else {
                if (!checkMethod(method)) {
                    /* Method was not "GET" or "HEAD". */
                    if (d_FLAG) {
                        (void)printf("405 Method Not Allowed");
                    }
                    free(method);
                    free(path);
                    free(protocol);
                    r_response.statusCode = 405;
                    r_response.path = "";
                    r_response.response = "Method Not Allowed";
                    return r_response;
                } else if (!checkProtocol(protocol)) {
                    /* Method was not "HTTP/1.0" or "HTTP/0.9". */
                    if (d_FLAG) {
                        (void)printf("415 Unsupported Media Type");
                    }
                    free(method);
                    free(path);
                    free(protocol);
                    r_response.statusCode = 415;
                    r_response.path = "";
                    r_response.response = "Unsupported Media Type";
                    return r_response;
                }
                char* updated_path;
                if ((updated_path = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
                    if (d_FLAG) {
                            (void)printf("malloc: %s\n", strerror(errno));
                    }
                    free(method);
                    free(path);
                    free(protocol);
                    r_response.statusCode = 500;
                    r_response.path = "";
                    r_response.response = "Internal Server Error";
                    return r_response;
                } 
                updated_path = checkPath(path);
                if (!isPrefix(updated_path, "/")) {
                    /* checkPath returned an error code; propagate it. */
                    unsigned long i;
                    for (i = 0; i < sizeof(valid_status_codes) / FIELD_SIZE; i++) {
                        if (isPrefix(updated_path, valid_status_codes[i])) {
                            r_response.statusCode = atoi(valid_status_codes[i]);
                            r_response.path = "";
                            r_response.response = updated_path;
                            return r_response;
                        }
                    }
                    r_response.statusCode = 500;
                    r_response.path = "";
                    r_response.response = updated_path;
                    return r_response;
                } else if (d_FLAG) {
                    (void)printf("The resolved path is: %s\n", updated_path);
                }
                path = strdup(updated_path);                
            }
        } else {
            /* (Header) Anything other than the first line. */
            //printf("%d\n", strncmp(line, msgEnd, 1));
            if (strncmp(line, msgEnd, 1) != 0) {
                headerVal = getHeaderContent(line);
                if (headerVal != 0) {
                    if (headerVal == 400) {
                        if (d_FLAG) {
                            (void)printf("400 Bad Request\n");
                        }
                        r_response.statusCode = 400;
                        r_response.path = "";
                        r_response.response = "Bad Request";
                    }
                    if (headerVal == 431) {
                        if (d_FLAG) {
                            (void)printf("431 Request Header Fields Too Large\n");
                        }
                        r_response.statusCode = 431;
                        r_response.path = "";
                        r_response.response = "Request Header Fields Too Large";
                    }
                    return r_response;
                }
            } else {
                printf("Ignored header\n");
            }
        }
        n++;
    }
    r_response.statusCode = 200;
    r_response.mtime = mtime;
    r_response.path = path;
    r_response.response = "OK";
    return r_response;
}
