#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sws.h"

int allocate_fd(struct addrinfo *p);
int socket_select();

#endif
