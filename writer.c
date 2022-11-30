#include "writer.h"

void writer(char* err, int client_fd){
    
    /* Determine what to response based on what reader() return */
    printf("reader(): %s\n", err);

    /* Initialize the http response */
    struct response r = {
        "HTTP/1.0",
        "200","OK",
        "Date: Later",
        "Server: Later",
        "Last-Modified: Later",
        "Content-Type: Later",
        "Content-Length: Later",
        "body-> Hello World"
        };

    /* Send the http response */
    char* result;
    int size = asprintf(&result, "%s %s %s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n\r\n%s",
    r.http_version, 
    r.status_code,
    r.status_message,
    r.date,
    r.server,
    r.last_modified,
    r.content_type,
    r.content_length,
    r.body);
    char response[size + 1];
    strlcpy(response, result, sizeof(response));
    // printf("Response:\n%s\n", response);
    send_response(client_fd, response, sizeof(response));

}
void send_response(int client_fd, void *response, size_t length)
{
    char *buf = (char*) response;
    while (length > 0)
    {
        int i = send(socket, buf, length, 1);
        if (i < 1) return false;
        buf += i;
        length -= i;
    }
    return true;
}

char* response_string(struct response r){
    char* result;
    /* First Line: HTTP/1.0 200 OK*/
    int size = asprintf(&result, "%s %s %s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n\r\n%s",
    r.http_version, 
    r.status_code,
    r.status_message,
    r.date,
    r.server,
    r.last_modified,
    r.content_type,
    r.content_length,
    r.body);
    puts(result);
    return result;

}