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
            
            reader_response r_response;
            // Reader
            r_response = reader(client_fd);
            printf("[reader return]%s\n\n", r_response.response);

            // Writer: Return a Hello world just for showcase
            writer(r_response.response, client_fd, r_response.path);
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

static const char *rfc1123_date = "%a, %d %B %Y %T %Z";
static const char *rfc850_date = "%a, %d-%B-%y %T %Z";
static const char *ansic_date = "%a %B %d %T %Y";

static time_t mtime = 1;

int
isValidDate(char *date)
{
    struct tm tm;
    int ret = 0;
    char *s;
    memset(&tm, 0, sizeof(tm));
    
    if ((s = strptime(date, rfc1123_date, &tm)) != NULL)
        ret = 1;
    else if ((s = strptime(date, rfc850_date, &tm)) != NULL)
        ret = 1;
    else if ((s = strptime(date, ansic_date, &tm)) != NULL)
        ret = 1;    
    if ((mtime = mktime(&tm)) == -1) {
        (void)fprintf(stderr, "mktime() %s\n", strerror(errno));
        return 0;
    }
    return ret;
}

void
getHeaderContent(char *line) {
    char header[64], field[64];
    if (sscanf(line," %[^: ] : %[^\t]", header, field) < 2) {
        return;        
    }
    if (strcmp(header, "If-Modified-Since") == 0) {
        (void)isValidDate(field);
    } 
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
            (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
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
                (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
            }
            free(prefix_root);
            return "500 Internal Server Error";
        }
    }

    if ((updated_path = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
        if (d_FLAG) {
            (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
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
                (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
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
                (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
            }
            free(prefix_root);
            free(newpath);
            free(updated_path);
            return "500 Internal Server Error";
        } 

        part = strtok(strdup(newpath), "/");
        if ((user = getpwnam(part+1)) == NULL) {
            if (d_FLAG) {
                (void)fprintf(stderr, "getpwnam: %s\n", strerror(errno));
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
                (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
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
                        (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
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
                        (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
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
            (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
        }
        free(prefix_root);
        free(newpath);
        free(updated_path);
        return "500 Internal Server Error";
    } else if (realpath(updated_path, real_updated) == NULL) {
        if (d_FLAG) {
            (void)fprintf(stderr, "realpath of %s: %s\n", updated_path, strerror(errno));
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
    if((n = write(logFD, logging_buffer, sizeof(logging_buffer))) == -1){
        if (d_FLAG) {
            (void)printf("Error while logging into file: %s\n", strerror(errno));
        }
        exit(1);
    }
}

reader_response
reader(int fd) {
    char buf[BUFSIZE];
    char* method;
    char* part;
    char* path;
    char* protocol;
    int index;

    reader_response r_response;

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
            if (d_FLAG) {
                (void)printf("[First Line]\n");
            }
            part = strtok(line, " ");
            index = 0;
            while (part != NULL) {
                if (index == 0) {
                    if ((method = strndup(part, 5)) == NULL) {
                        (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
                    }
                } else if (index == 1) {
                    if ((path = strdup(part)) == NULL) {
                        free(method);
                        (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
                    }
                } else if (index == 2) {
                    if ((protocol = strdup(part)) == NULL) {
                        free(method);
                        free(path);
                        (void)fprintf(stderr, "strdup: %s\n", strerror(errno));
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
                if (index >= 1) {
                    free(method);
                }
                if (index >= 2) {
                    free(path);
                }
                if (index >= 3) {
                    free(protocol);
                }
                if (d_FLAG)
                    (void)printf("400 Bad Request");
                
                r_response.path = "";
                r_response.response = "400 Bad Request";
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
                    r_response.path = "";
                    r_response.response = "405 Method Not Allowed";
                    return r_response;
                } else if (!checkProtocol(protocol)) {
                    /* Method was not "HTTP/1.0" or "HTTP/0.9". */
                    if (d_FLAG) {
                        (void)printf("415 Unsupported Media Type");
                    }
                    free(method);
                    free(path);
                    free(protocol);
                    r_response.path = "";
                    r_response.response = "415 Unsupported Media Type";
                    return r_response;
                }
                char* updated_path;
                if ((updated_path = (char *)malloc(sizeof(char)*(PATH_MAX+1))) == NULL) {
                    if (d_FLAG) {
                            (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
                    }
                    free(method);
                    free(path);
                    free(protocol);
                    r_response.path = "";
                    r_response.response = "500 Internal Server Error";
                    return r_response;
                } 
                updated_path = checkPath(path);
                if (!isPrefix(updated_path, "/")) {
                    /* checkPath returned an error code; propagate it. */
                    r_response.path = "";
                    r_response.response = updated_path;
                    return r_response;
                } else if (d_FLAG) {
                    (void)printf("The resolved path is: %s\n", updated_path);
                }
                path = strdup(updated_path);
                /* Logging if l flag is given 
                if(l_FLAG) {
                    logging(method, path, protocol, response);
                }
                */
                
            }
        } else {
            /* (Header) Anything other than the first line. */
            getHeaderContent(line);
        }
        // line = strtok(NULL, "\r\n");
        n++;
    }
    r_response.mtime = mtime;
    r_response.path = path;
    r_response.response = "200 OK";
    return r_response;
}
