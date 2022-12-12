#ifndef READER_HEADER
#define READER_HEADER

#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>

#include "writer.h"

#define BUFSIZE 1024
#define FIELD_SIZE 64
#define NUMLINES 1024

bool checkProtocol(char* protocol);
bool checkMethod(char* method);
bool isPrefix(char* string, char* prefix);
char* checkPath(char* path);
int getHeaderContent(char *line);
int isValidDate(char *date);
int modified(char* path);
reader_response reader(int fd);
void handle_socket(int server_fd);
void updatePath(char** updated_path, char* path, char* initial);

#endif
