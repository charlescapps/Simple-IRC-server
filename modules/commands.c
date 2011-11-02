/*
 * commands.c
 *
 *  Created on: Oct 18, 2011
 *      Author: charles
 */

const int BUFFER_SIZE = 512; 

//Commands from client
const char* CONNECT_CMD = "+CONNECT"; 
const char* QUIT_CMD = "+QUIT"; 
const char* MSG_CMD = "+MSG"; 
const char* LIST_CMD = "+LIST"; //list names of ppl in channel
const char* ROOMS_CMD = "+ROOMS";
const char* HELP_CMD = "+HELP"; 

//Commands to client
const char* PRINT_CMD = "+PRINT"; 
const char* SET_PROMPT_CMD = "+PROMPT"; 
const char* FAIL_CMD = "+FAIL"; 

