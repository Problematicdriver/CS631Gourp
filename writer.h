#ifndef WRITER_HEADER
#define WRITER_HEADER

#include <strings.h>
#include <dirent.h>
#include <magic.h>

#include "sws.h"

typedef struct response
{
    char* http_version;
    char* status_code;
    char* status_message;
    char* date;
    char* server;
    char* last_modified;
    char* content_type;
    char* content_length;
    char* body;
} repsonse;

typedef struct reader_response {
    char* path;
    char* firstLine;
    int statusCode;
    char* response;
    time_t mtime;
    char* requestTime;
    char* remoteIp;
} reader_response;

void writer(reader_response r_response, int client_fd);
void send_response(int client_fd, void *response, size_t length);
char* r_body(char* path);
char* file_content(char* path);
char* dir_content(char* path);
void logging(char* remoteAddress, char* reqestedTime, char* firstLineOfRequest, int status, int responseSize);

#endif
