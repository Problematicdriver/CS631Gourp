// Testing this is joe
// Hey

#include "socket.h"
#include "sws.h"

#define OPTSTRING "c:dhi:l:p:"
#define DEFAULT_PORT "8080"

bool c_FLAG;
bool d_FLAG;
bool h_FLAG;
bool i_FLAG;
bool l_FLAG;
bool p_FLAG;
char* docroot;
char* cgidir;
char* hostname;
char* logFile;
char* port;
int ch;
int _logFD;
struct stat docrootSB;
struct stat cgidirSB;


int
main(int argc, char** argv) {
    c_FLAG = false;
    d_FLAG = false;
    h_FLAG = false;
    i_FLAG = false;
    l_FLAG = false;
    p_FLAG = false;
    hostname = "localhost";
    port = DEFAULT_PORT;

    while ((ch = getopt(argc, argv, OPTSTRING)) != -1){
        switch (ch) {
            case 'c':
                c_FLAG = true;
                cgidir = optarg;
                break;
            case 'd':
                d_FLAG = true;
                break;
            case 'h':
                h_FLAG = true;
                break;
            case 'i':
                i_FLAG = true;
                hostname = optarg;
                break;
            case 'l':
                l_FLAG = true;
                logFile = optarg;
                break;
            case 'p':
                p_FLAG = true;
                port = optarg;
                break;
            default:
                break;
        }
    }

    if (
        h_FLAG ||
        (c_FLAG && cgidir==NULL) ||
        (i_FLAG && hostname==NULL) ||
        (l_FLAG && logFile==NULL) ||
        (p_FLAG && port==NULL) ||
        (optind+1 != argc)
    ) {
        (void)printf("Usage: %s [ −dh] [ −c dir] [ −i address] [ −l file] [ −p port] dir\n", argv[0]);
        return EXIT_SUCCESS;
    }

    if (d_FLAG) {
        (void)printf("Debugging mode active.\n");
    }

    /* Confirm the entered docroot is a valid directory. */
    docroot = argv[argc-1];
    char* real_docroot;
    if ((real_docroot = (char *)malloc(sizeof(char)*PATH_MAX)) == NULL) {
        (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    } else if (realpath(docroot, real_docroot) == NULL) {
        (void)fprintf(stderr, "realpath of %s: %s\n", docroot, strerror(errno));
        return EXIT_FAILURE;
    } else {
        if (d_FLAG) {
            (void)printf("The realpath of %s is %s\n", docroot, real_docroot);
        }
    }

    if (stat(real_docroot, &docrootSB) == -1) {
        (void)fprintf(stderr, "Can't stat %s", real_docroot);
        return EXIT_FAILURE;
    } else if (!S_ISDIR(docrootSB.st_mode)) {
        (void)fprintf(stderr, "%s not a directory.\n", real_docroot);
        return EXIT_FAILURE;
    } else if (d_FLAG) {
        (void)printf("%s is a directory.\n", real_docroot);
    }

    /* Confirm the entered cgidir is a valid directory. */
    if (c_FLAG) {
        char* real_cgidir;
        if ((real_cgidir = (char *)malloc(sizeof(char)*PATH_MAX)) == NULL) {
            (void)fprintf(stderr, "malloc: %s\n", strerror(errno));
            return EXIT_FAILURE;
        } else if (realpath(cgidir, real_cgidir) == NULL) {
            (void)fprintf(stderr, "realpath of %s: %s\n", cgidir, strerror(errno));
            return EXIT_FAILURE;
        } else {
            if (d_FLAG) {
                (void)printf("The realpath of %s is %s\n", cgidir, real_cgidir);
            }
        }

        if (stat(real_cgidir, &cgidirSB) == -1) {
            (void)fprintf(stderr, "Can't stat %s", real_docroot);
            return EXIT_FAILURE;
        } else if (!S_ISDIR(cgidirSB.st_mode)) {
            (void)fprintf(stderr, "%s: %s not a directory.\n", argv[0], real_cgidir);
            return EXIT_FAILURE;
        } else if (d_FLAG) {
            (void)printf("%s is a directory.\n", real_cgidir);
        }
    }


    if (l_FLAG) {
        if ((_logFD = open(logFile, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR)) == -1){
            if (d_FLAG) {
                (void)fprintf(stderr, "Error: create/open log file: %s\n", strerror(errno));
            }
            return EXIT_FAILURE;
        }
    }

    // check it is a valid IP?
    if (i_FLAG) {
        //Check Ipv4
        struct in_addr addr4;
        if(inet_pton(PF_INET, hostname, (void *)&addr4) != 1) {
            //Check Ipv6
            struct in6_addr addr6;
            if(inet_pton(PF_INET6, hostname, (void *)&addr6) != 1) {
                (void)fprintf(stderr, "Error: invalid IP.\n");
                return EXIT_FAILURE;
            }
        }

        
    }
    
    // check it is a valid port?
    if (p_FLAG) {
        // potentially 1025 to avoid priviliedged ports
        if (atoi(port) < 0 || atoi(port) > 65536) {
            (void)fprintf(stderr, "Error: invalid port number.\n");
            return EXIT_FAILURE;
        }
        printf("%s\n", port);
    }

    if (!d_FLAG) {
        if (daemon(0, 0) == -1) {
            (void)fprintf(stderr, "daemon: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }
    // call to create socket
    // create_socket();
    int value;
    if ((value = socket_select()) != 0) {

        // if (d_FLAG) {
        fflush(stderr);
        (void)fprintf(stderr, "create_socket()\n");
        // }
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
