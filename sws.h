#ifndef SWS_HEADER
#define SWS_HEADER

#include <sys/socket.h>
#include <sys/stat.h>

#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


extern bool c_FLAG;
extern bool d_FLAG;
extern bool h_FLAG;
extern bool i_FLAG;
extern bool l_FLAG;
extern bool p_FLAG;

extern char *hostname;
extern char *port;

extern int _logFD;

#endif
