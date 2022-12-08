#include "writer.h"

char* get_last_modified(char *path);
char* get_date();
char* get_content_type(char *path);

void writer(reader_response r_response, int client_fd){

    /* Determine what to response based on what reader() return */
    printf("reader(): %s\n", r_response.response);

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

    /* filling in reponse */
    char header[128], *head_content;
    if ((head_content = get_date()) == NULL) {
        // error status
        printf("get_date()\n");
    }
    if (snprintf(header, 128, "Date: %s", head_content) < 0) {
        printf("hahahhahahhahhah)\n");
        fprintf(stderr, "sprintf %s\n", strerror(errno));
    }
    printf("%s\n", header);
    r.date = header;

    if ((head_content = get_last_modified(r_response.path)) == NULL) {
        // error status
    }
    if (snprintf(header, 128, "Last-Modified: %s", head_content) < 0) {
        fprintf(stderr, "sprintf %s\n", strerror(errno));
    }
    r.last_modified = header;
    
    if ((head_content = get_content_type(r_response.path)) == NULL) {
        // error status
    }
    if (snprintf(header, 128, "Content-Type: %s", head_content) < 0) {
        fprintf(stderr, "sprintf %s\n", strerror(errno));
    }
    r.content_type = header;

    printf("[/etc/passwd]:\n%s\n",r_body(r_response.path));
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
    if(l_FLAG && !d_FLAG) {
        logging(r_response.remoteIp, r_response.requestTime, r_response.firstLine, r_response.statusCode, size);
    }
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
    if((n = write(logFD, logging_buffer, sizeof(logging_buffer))) == -1){
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
		if (strncmp(dirp->d_name, ".", strlen(".") != 0)) 
            size += asprintf(&str, "%s%s\n", str,strdup(dirp->d_name));
	}
	printf("Size: %d\n", size);
	
    char* r = malloc(sizeof(char) * size);
    strlcpy(r, str, size);
	(void)closedir(dp);
	return r;
}

char*
get_last_modified(char *path)
{
    struct stat sb;
    char *s;

    if (lstat(path, &sb) < 0) {
        fprintf(stderr, "lstat() %s\n", strerror(errno));
    }
    if ((s = (asctime(gmtime(&(sb.st_mtime))))) == NULL) {
        fprintf(stderr, "asctime() %s\n", strerror(errno));
    }
    return s;
}

char*
get_date() {
    time_t now;
    char *s;
    if ((now = time(0)) == (time_t)-1) {
        fprintf(stderr, "time() %s\n",  strerror(errno));
    }
    if ((s = (asctime(gmtime(&now)))) == NULL) {
        fprintf(stderr, "asctime() %s\n", strerror(errno));
    }
    return s;
}

char*
get_content_type(char *path) {
    const char *mime;
    magic_t magic;
    if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
        fprintf(stderr, "magic_open() %s\n", strerror(errno));
    } 
    if (magic_load(magic, NULL) < 0) {
        fprintf(stderr, "magic_load() %s\n", strerror(errno));
    }
    if ((mime = magic_file(magic, path)) == NULL) {
        fprintf(stderr, "magic_file() %s\n", strerror(errno));
    }
    magic_close(magic);
    return strdup(mime);
}