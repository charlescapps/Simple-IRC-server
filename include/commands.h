#ifndef _IRC_COMMANDS
#define _IRC_COMMANDS

const int BUFFER_SIZE; 

//Commands from client
const char* CONNECT_CMD; 
const char* QUIT_CMD; 
const char* MSG_CMD; //User doesn't have to type +MSG, appended by program when in a valid chat room 
const char* LIST_CMD; //list names of ppl in channel
const char* ROOMS_CMD;
const char* HELP_CMD; 

//Commands to client
const char* PRINT_CMD; 
const char* SET_PROMPT_CMD; 
const char* FAIL_CMD; 


#endif
