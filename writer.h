#include <strings.h>
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
};

void writer(char* err, int client_fd);
char* response_string(struct response r);
bool send_all(int socket, void *buffer, size_t length);