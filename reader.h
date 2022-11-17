#ifndef READER_HEADER
#define READER_HEADER

#include <sys/param.h>
#include <sys/wait.h>

#include <stdbool.h>
#include <stdlib.h>

#include "sws.h"

#define BUFSIZE 1024

void handle_socket(int server_fd);
bool checkProtocol(char* protocol);
bool checkMethod(char* method);
char* checkPath(char* path);
char* reader(int fd);

#endif
