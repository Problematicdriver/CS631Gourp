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

void handle_socket(int server_fd);
bool checkProtocol(char* protocol);
bool checkMethod(char* method);
bool isPrefix(char* string, char* prefix);
int modified(char* path);
int isValidDate(char *date);
int getHeaderContent(char *line);
void updatePath(char** updated_path, char* path, char* initial);
char *checkPath(char* path);
reader_response reader(int fd);

#endif
