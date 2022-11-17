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
char* hostname;
char* cgiDir;
char* logFile;
char* port;
int ch;
int _logFD;

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
                cgiDir = optarg;
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
        (c_FLAG && cgiDir==NULL) ||
        (i_FLAG && hostname==NULL) ||
        (l_FLAG && logFile==NULL) ||
        (p_FLAG && port==NULL)
    ) {
        (void)printf("Usage: %s [ −dh] [ −c dir] [ −i address] [ −l file] [ −p port] dir\n", argv[0]);
        return EXIT_SUCCESS;
    }

    if (l_FLAG) {
        // open/create the file O_CREAT O_APPEND
        if ((_logFD = open(logFile, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR)) == -1){
            if (d_FLAG) {
                (void)fprintf(stderr, "Error: create/open log file: %s\n", strerror(errno));
            }
            return EXIT_FAILURE;
        }
    }

    /* Debugging information. */
    if (d_FLAG) {
        printf("Debugging mode active.\n");

        // check it is a directory?
        if (c_FLAG) {
            printf("%s\n", cgiDir);
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
