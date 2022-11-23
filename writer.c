#include "writer.h"

void writer(reader_response r_response, int client_fd){
    if (d_FLAG) {
        (void)printf("reader returned(): %s\n", r_response.response);    
    }
    
    struct response r = response_content(r_response.statusCode, r_response.path, r_response.cgi);
    char* result;
    int size = asprintf(&result, "%s %s %s\r\n%s%s%s%s%s\r\n\r\n%s",
        r.http_version, 
        r.status_code,
        r.status_message,
        r.date,
        r.last_modified,
        r.server,
        r.content_type,
        r.content_length,
        r.body);
    char response[size + 1];
    strlcpy(response, result, size+1);
    if (d_FLAG) {
        (void)printf("Response:\n%s\n", response);    
    }
    send_response(client_fd, response, size+1);
    if(l_FLAG && !d_FLAG) {
        logging(r_response.remoteIp, r_response.requestTime, r_response.firstLine, r_response.statusCode, size);
    }
    close(client_fd);
}

void
logging(char* remoteAddress, char* reqestedTime, char* firstLineOfRequest, int status, int responseSize) {
    char* logging_buffer;
    int n;

    if ((logging_buffer = (char *)malloc(sizeof(char)*BUFSIZ)) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
        exit(1);
    } 
    sprintf(logging_buffer, "%s %s %s %d %d", remoteAddress, reqestedTime, firstLineOfRequest, status, responseSize);
    if((n = write(logFD, logging_buffer, strlen(logging_buffer))) == -1){
        if (d_FLAG) {
            (void)printf("Error while logging into file: %s\n", strerror(errno));
        }
        exit(1);
    }
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
r_body(char* path, bool cgi){
    struct stat path_stat;
    stat(path, &path_stat);
    if (S_ISDIR(path_stat.st_mode)) {
        char* index_file;
        asprintf(&index_file, "%s%s", path, "/index.html");
        struct stat index_stat;
        stat(index_file, &index_stat);
        /* 
         * If index.html exist, serve index.html 
         * else response all file names in DIR 
         */
        if (S_ISREG(index_stat.st_mode)) {
            return file_content(index_file);
        } else {
            return dir_content(path);
        }
        
    } else {
	if(cgi){
		return cgi_content(path);
	}
        return file_content(path);
    }
}

char* 
file_content(char* path){
    FILE* fp = fopen(path, "r");
    char buf[BUFSIZ];
    int size = 0;
	char* str = "";
    char* r;
    while (fgets(buf, BUFSIZ, fp) != NULL) {
        size += asprintf(&str, "%s%s", str, buf);
    }

    if ((r = malloc(sizeof(char) * size)) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
        return NULL;
    }
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
		if (strncmp(dirp->d_name, ".", strlen(".") != 0)) 
            size += asprintf(&str, "%s%s\n", str,strdup(dirp->d_name));
	}
	printf("Size: %d\n", size);
	
    char *r;
    if ((r = malloc(sizeof(char) * size)) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
        return NULL;
    }
    strlcpy(r, str, size);
	(void)closedir(dp);
	return r;
}
char* cgi_content(char* filepath){

    FILE *fp;
    char buf[BUFSIZ];
    char *path = "/bin/sh";
    char *filename = filepath;
    asprintf(&path, "%s %s", path, filename);
    /* Open the command for reading. */
    fp = popen(path, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return NULL;
    }

    int size = 0;
	char* str = "";
    /* Read the output a line at a time - output it. */
    while (fgets(buf, BUFSIZ, fp) != NULL) {
        size += asprintf(&str, "%s%s\n", str, buf);
    }

    /* close */
    pclose(fp);

    char *r;
    if ((r = malloc(sizeof(char) * size)) == NULL) {
        if (d_FLAG) {
            (void)printf("malloc: %s\n", strerror(errno));
        }
        return NULL;
    }
    strlcpy(r, str, size);
    return r;
}

struct response
response_content(int code, char* path, bool cgi){
    char body[BUFSIZ];
    char *content_length = "Content-Length:";
    char *content_type = "Content-Type:";
    char *last_modified = "Last-Modified:";
    char *date = "Date:";
    
    asprintf(&date, "%s %s", date, get_time());

    struct response r = {
        "HTTP/1.0",
        "","",                  // 200, OK
        date,                     // date
        "Server: sws\r\n",          
        "",                     // Last-Modified
        "",                     // Content-Type
        "",                     // Content-Length
        ""                      // Body
        };
    
    char *modified_date = get_last_modified(path);
    char *type = get_type(path);
    if (code == 200 && (modified_date == NULL || type == NULL || date == NULL)) {
        code = 500;
    }

    switch (code) {
    case 200:
        
        r.status_code = "200";
        r.status_message = "OK";
        r.body = r_body(path, cgi);
        if(r.body == NULL){
            goto server_error;
        } 
        asprintf(&last_modified, "%s %s", last_modified, modified_date);
        r.last_modified = last_modified;
        asprintf(&content_length, "%s %ld", content_length, strlen(r.body));
        r.content_length = content_length;
        asprintf(&content_type, "%s %s", content_type, type);
        r.content_type = content_type;

        break;
    case 304:
        r.status_code = "304";
        r.status_message = "Not Modified";
        r.body = "304 Not Modified";
 
        strcpy(body, "304 Not Modified");

        asprintf(&content_length, "%s %ld", content_length, strlen(r.body));
        r.content_length = content_length;
        r.content_type = "Content-Type: text/html\r\n";
        break;
    case 400:
        r.status_code = "400";
        r.status_message = "Bad Request";
        r.body = "400 Bad Request";

        strcpy(body, "400 Bad Request");
        asprintf(&content_length, "%s %ld", content_length, strlen(body));
        r.content_length = content_length;
        r.content_type = "Content-Type: :text/html\r\n";
        break;
    case 404:
        r.status_code = "404";
        r.status_message = "Not Found";
        r.body = "404 Not Found";
	
        strcpy(body, "404 Not Found");
        asprintf(&content_length, "%s %ld", content_length, strlen(body));
        r.content_length = content_length;
        r.content_type = "Content-Type: text/html\r\n";
        break;
    case 405:
        r.status_code = "405";
        r.status_message = "Method Not Allowed";
        r.body = "405 Method Not Allowed";

        strcpy(body, "405 Method Not Allowed");

        asprintf(&content_length, "%s %ld", content_length, strlen(body));
        r.content_length = content_length;
        r.content_type = "Content-Type: text/html\r\n";
        break;
    case 415:
        r.status_code = "415";
        r.status_message = "Unsupported Media Type";
        r.body = "415 Unsupported Media Type";

        strcpy(body, "415 Unsupported Media Type");
        asprintf(&content_length, "%s %ld", content_length, strlen(body));
        r.content_length = content_length;
        r.content_type = "Content-Type: text/html\r\n";
        break;
    case 500:
    server_error:
        r.status_code = "500";
        r.status_message = "Internal Server Error";
        r.body = "500 Internal Server Error";

        strcpy(body, "500 Internal Server Error");
        asprintf(&content_length, "%s %ld", content_length, strlen(body));
        r.content_length = content_length;
        r.content_type = "Content-Type: text/html\r\n";
        break;
    case 503:
        r.status_code = "503";
        r.status_message = "Service Unavailable";
        r.body = "503 Service Unavailable";

        strcpy(body, "503 Service Unavailable");
        asprintf(&content_length, "%s %ld", content_length, strlen(body));
        r.content_length = content_length;
        r.content_type = "Content-Type: text/html\r\n";
        break;
    }
    return r;
}

char*
get_last_modified(char *path)
{
    struct stat sb;
    char *s, *t;

    if (lstat(path, &sb) < 0) {
        fprintf(stderr, "lstat() %s\n", strerror(errno));
        return NULL;
    }
    if ((s = (asctime(gmtime(&(sb.st_mtime))))) == NULL) {
        fprintf(stderr, "asctime() %s\n", strerror(errno));
        return NULL;
    }
    int len = strlen(s);
    s[len - 1] = '\0';
    asprintf(&t, "%s\r\n", s);
    return t;
}

char*
get_time() {
    time_t now;
    char *s, *t;
    if ((now = time(0)) == (time_t)-1) {
        fprintf(stderr, "time() %s\n",  strerror(errno));
        return NULL;
    }
    if ((s = (asctime(gmtime(&now)))) == NULL) {
        fprintf(stderr, "asctime() %s\n", strerror(errno));
        return NULL;
    }
    int len = strlen(s);
    s[len-1] = '\0';
    asprintf(&t, "%s\r\n", s);
    return t;
}

char*
get_type(char *path) {
    const char *mime;
    char *type;
    magic_t magic;
    if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
        if (d_FLAG) {
            (void)printf("magic_open() %s\n", strerror(errno));    
        }
        return NULL;
    } 
    if (magic_load(magic, NULL) < 0) {
        if (d_FLAG) {
            (void)printf("magic_load() %s\n", strerror(errno));
        }
        return NULL;
    }
    if ((mime = magic_file(magic, path)) == NULL) {
        if (d_FLAG) {
            (void)printf("magic_file() %s\n", strerror(errno));
        }
        return NULL;
    }
    magic_close(magic);
    asprintf(&type, "%s\r\n", mime);
    return type;
}
