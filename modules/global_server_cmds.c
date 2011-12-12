#include "../include/global_server_cmds.h"


bool valid_charset(char* test) {

	if (test == NULL) {
		printf("In valid_charse: NULL string received.\n"); 
		return false; 
	}
	int len = strlen(test), i = 0; 

	for (; i < len; i++) {
		if (!char_in_charset(test[i]))
			return false;
	}
	return true; 
}


bool char_in_charset(char c) {

	if ('a' <= c && c <= 'z')
		return true; 

	if ('A' <= c && c <= 'Z')
		return true; 

	if ('0' <= c && c <= '9')
		return true; 

	if (c == '-' || c == '_' || c == '*' || c == '&' || c == '#')
		return true; 

	return false; 

}

char* get_next_cmd(int socket, char* partial_cmd, bool* err) {
	int num_bytes = -1, null_char_loc = -1;
	char buffer[BUFFER_SIZE]; 	
	char* next_cmd;
	*err = false; 
	
	if ((num_bytes = recv(socket, buffer, BUFFER_SIZE, 0)) <= 0) {
		*err = true; 
		return NULL; 
	}	

	null_char_loc = contains_null_char(buffer, num_bytes);
   //We didn't find the end of a command	
	if (null_char_loc < 0) {
		strncat(partial_cmd, buffer, num_bytes); //concatenate buffer onto current partial command 
		return NULL; //NULL means no new command
	}
	else { //We found a null char, hence end of a command
		int len = strlen(buffer); 
		strncat(partial_cmd, buffer, len); 
		next_cmd = (char*)malloc(sizeof(char)*(strlen(partial_cmd)+1)); 
		strcpy(next_cmd, partial_cmd); 
		partial_cmd[0]='\0'; //Reset partial command

		//If we received 2 commands simultaneously
		if (len + 1 < num_bytes) {
			strncpy(partial_cmd, buffer + (len + 1), num_bytes - (len + 1)); 
		}
		return next_cmd; 
	}
}

int contains_null_char(char* str, int num_bytes) {
	int i = 0, pos = -1; 
	for (; i<num_bytes; i++) {
		if (str[i] == '\0') {
			pos = i; 
			break; 
		}
	}
	return pos;
}
