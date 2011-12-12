#ifndef _GLOBAL_SERVER_CMDS
#define _GLOBAL_SERVER_CMDS

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "constants.h"

//Check if a nick or room name uses only allowed chars
bool valid_charset(char* test); 

//Checks an individual char
bool char_in_charset(char c); 

//Reads from socket to get the next command from the user
//Caller must have a buffer with partial command
//Returns new command (dynamically allocated) if we got a full cmd
//Otherwise, returns NULL indicating only received partial cmd
char* get_next_cmd(int socket, char* partial, bool* err); 

//Just a helper function to check if the data received had a null char
//Must use this since all std. library functions assume the string
//is already null-terminated
int contains_null_char(char* str, int num_bytes); 

#endif
