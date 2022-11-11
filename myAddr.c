#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <netinet/in.h>

int
main()
{
    int required_family = AF_INET;
    struct ifaddrs *myaddrs, *ifa;
    getifaddrs(&myaddrs);
    char host[256], port[256];
        
    for(ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        int family = ifa->ifa_addr->sa_family;
        if (family == required_family && ifa->ifa_addr) {
            int ret = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                    sizeof(struct sockaddr_in6),
                    host, sizeof(host), port, sizeof(port),
                     NI_NUMERICHOST | NI_NUMERICSERV);
            if (0 == ret)
                puts(host);
        }
    }
}
