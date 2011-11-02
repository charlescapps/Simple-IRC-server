#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netdb.h"
#include "errno.h"

/*
 *Function to open a socket and bind it.
 *Returns the file descriptor for the socket. 
 */
int start_server(char* socket_str); 

int start_listening(int socket_fd); 
