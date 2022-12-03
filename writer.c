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
        "body-> Hello World!"
        };
    printf("[/etc/passwd]:\n%s\n",r_body("/etc/passwd"));
    // r_body = body;
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
    printf("Response:\n%s\n", response);
    send_response(client_fd, response, sizeof(response));

}

void send_response(int client_fd, void *response, size_t length)
{
    char *buf = (char*) response;
    while (length > 0)
    {
        int i = send(client_fd, buf, length, 1);
        if (i < 1) break;
        buf += i;
        length -= i;
    }
}

char* 
r_body(char* path){
    struct stat path_stat;
    stat(path, &path_stat);
    /* DIR */
    if (S_ISDIR(path_stat.st_mode))
    {
        char* index_file;
        asprintf(&index_file, "%s%s", path, "/index.html");
        struct stat index_stat;
        stat(index_file, &index_stat);
        /* If index.html exist, response index.html */
        /* else response all file names in DIR */
        if (S_ISREG(index_stat.st_mode)) {
            return file_content(index_file);
        } else {
            /* DIR */
            return dir_content(path);
        }
        
    } else {
        /* NOT DIR, file_content */
        return file_content(path);
        /* cgi-bin not implemented yet */
    }
    return "fuck";
}

char* 
file_content(char* path){
    FILE* fp = fopen(path, "r");
    char buf[BUFSIZ];

    int size = 0;
	char* str = "";
    while (fgets(buf, BUFSIZ, fp) != NULL)
    {
        size += asprintf(&str, "%s%s", str, buf);
    }

    char* r = malloc(sizeof(char) * size);
    strlcpy(r, str, size);
    return r;
}

char*
dir_content(char* path) {

	DIR *dp;
	struct dirent *dirp;

	if ((dp = opendir(path)) == NULL) {
		fprintf(stderr, "Unable to open '%s': %s\n",
					path, strerror(errno));
		exit(EXIT_FAILURE);
	}
	int size = 0;
	char* str = "";
	while ((dirp = readdir(dp)) != NULL) {
		size += asprintf(&str, "%s%s\n", str,strdup(dirp->d_name));
	}
	printf("Size: %d\n", size);
	
    char* r = malloc(sizeof(char) * size);
    strlcpy(r, str, size);
	(void)closedir(dp);
	return r;
}
