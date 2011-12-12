#ifndef _IRC_COMMANDS
#define _IRC_COMMANDS

/*
 * These constants are defined in commands.c
 */

//Commands from client
const char* CONNECT_CMD; 
const char* QUIT_CMD; 
const char* MSG_CMD; //User doesn't have to type +MSG, appended by program when in a valid chat room 
const char* LIST_CMD; //list names of ppl in channel
const char* LIST_ALL_CMD; //list names of all ppl on server
const char* ROOMS_CMD;
const char* HELP_CMD; 
const char* JOIN_CMD; 

//Commands to client
const char* PRINT_CMD; 
const char* SET_PROMPT_CMD; 
const char* FAIL_CMD; 
const char* JOIN_SUCCESS_CMD; 

#endif
