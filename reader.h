#ifndef READER_HEADER
#define READER_HEADER

#include <sys/param.h>
#include <sys/wait.h>

#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "writer.h"
#define BUFSIZE 1024

typedef struct reader_response {
    char* path;
    char* response;
} reader_response;

void handle_socket(int server_fd);
bool checkProtocol(char* protocol);
bool checkMethod(char* method);
char* checkPath(char* path);
reader_response reader(int fd);

#endif
