#include "../include/chat_room.h"
/*
 * See chat_room.h for comments on what these functions do. 
 */

static const char* LOBBY_NAME = "LOBBY"; //Name of lobby room
static list* all_chat_rooms = NULL; //List of all chat rooms. Static for C-style data encapsulation
static chat_room* LOBBY = NULL; //LOBBY chat_room object

void init_chat_rooms() {

	if (all_chat_rooms == NULL) {
		all_chat_rooms = new_list((void(*)(void*))free_chat_room); 
	}

	if (LOBBY == NULL) {
		LOBBY = new_chat_room((char*)LOBBY_NAME); 
		add_to_list(all_chat_rooms, (void*)LOBBY); 
	}
}

chat_room* new_chat_room(char* name) { //Creates new chat room called 'name'. Assumes name is dynamically allocated already.

	chat_room* new_room = (chat_room*)malloc(sizeof(chat_room)); 
	new_room -> users_in_room = new_list((void(*)(void*)) free_user ); 
	new_room -> room_name = name; 
	new_room -> current_msg = NULL; 
	pthread_mutex_init(&(new_room->chat_room_mutex), NULL); 

	return new_room; 
}	

void free_chat_room(chat_room* c) {
	pthread_mutex_destroy(&(c->chat_room_mutex));
	free(c->room_name); 
	free(c); 
}

void add_user_to_lobby(user* u) { 

	char new_prompt[MAX_PROMPT_SIZE]; 
	char msg_buffer[BUFFER_SIZE]; 

	sprintf(new_prompt, "%s @ %s>", u->nick, LOBBY_NAME); 
	set_user_prompt(u, new_prompt); //Sends +PROMPT command to user to force change of prompt

	add_to_list(LOBBY -> users_in_room, (void*)u); 
	u->current_room = LOBBY; 

	print_raw_to_user(u, (char*)JOIN_SUCCESS_CMD); 
	sprintf(msg_buffer, "+PRINT [Server]: Connected as user '%s'.\n[Server]: Now entering lobby.\n \n", u->nick); 
	print_raw_to_user(u, msg_buffer); 

	chat_room_loop(u); 
}

void chat_room_loop(user* u) {
	char partial_cmd[BUFFER_SIZE]; //Required by get_next_cmd. Calling function must have buffer of partial command.
	char* next_cmd = NULL; 	
	bool err = false;	

	partial_cmd[0] = '\0'; 

	while (true) {

		if (next_cmd) {
			free(next_cmd); 
			next_cmd = NULL;
		}
		next_cmd = get_next_cmd(u->user_socket, partial_cmd, &err); 

		if (err) {
			printf("Error reading message from client.\nClosing socket #%d to client '%s'\nError: %s\n", u->user_socket, u->nick, strerror(errno)); 
			remove_user(u); 
			if (next_cmd) {
				free(next_cmd); 
			}
			pthread_exit(NULL); 
		}	
		else if (next_cmd == NULL) { //Haven't received full command
			continue; 	
		}
		else {
			trim_str(next_cmd); 
			printf("Message from client %s: %s\n", u->nick, next_cmd); 
			act_on_chat_cmd(next_cmd, u); 
		}

	}
}

void act_on_chat_cmd(char* cmd, user* u) {
	char* tok1 = NULL;
	char* tok2 = NULL;
	char* msg = NULL; 

	//Get the first token to determine which command we received.
	//Of course, strtok adds null char after each toekn is read, so keep this in mind
	tok1 = strtok(cmd, " "); 

	if (strcmp(tok1, MSG_CMD) == 0) { //+MSG
		msg = (char*)(&cmd[strlen(MSG_CMD)+1]); //Send the message as everything after +MSG
		trim_str(msg); //Trim since chat messages can't have endlines
		msg_all_users(msg, u->current_room, u->nick); 
	}
	else if (strcmp(tok1, JOIN_CMD)==0) { //+JOIN
		tok2 = strtok(NULL, " "); //Room name is the second token 
		add_user_to_room(u, tok2); 
	}
	else if (strcmp(tok1, LIST_CMD) == 0) {
		list_users_in_room(u); 	
	}
	else if (strcmp(tok1, LIST_ALL_CMD) == 0) {
		list_server_users(u); 
	}
	else if (strcmp(tok1, ROOMS_CMD) == 0) {
		list_all_rooms(u); 
	}
	else {
		printf("Received bad command from user '%s', sending fail msg\n", u->nick); 
		print_raw_to_user(u, (char*)FAIL_CMD); 
	}
}

void add_user_to_room(user* u, char* room_name) { //Adds user to room if it exists, otherwise creates new room and adds user

	if (room_name == NULL || strlen(room_name) <= 0) {
		printf("Error: Attempt to add user to NULL or empty room name.\n"); 
		return;
	}

	if (strcasecmp(room_name, u->current_room->room_name) == 0) {
		print_to_user(u, "[Server]: You are already in that chat room!"); 
		return; 
	}

	if (!valid_charset(room_name)) {
		print_to_user(u, "[Server]: Invalid room name!\n"
							"[Server]: Alphanumeric or '_', '-', '*', '&' required!\n "); 
		return; 
	}
	chat_room* room_to_enter; 
	char* new_room_name; //Will dynamically allocate to pass to new_room(char*)
	char msg_buffer[BUFFER_SIZE]; 
	char new_prompt[MAX_PROMPT_SIZE]; 

	sprintf(new_prompt, "%s @ %s>", u->nick, room_name); 
	set_user_prompt(u, new_prompt); //Set user's prompt

	if ( (room_to_enter = chat_room_exists(room_name))) { //Check if room exists. 
		remove_node(u->current_room->users_in_room, u); //Remove user from old chat room
		u->current_room = room_to_enter; 				//Set user's current room to the new room  
		add_to_list(room_to_enter->users_in_room, (void*)u); //Add user to new chat room's list of users 

		//If we successfully added user to room, send success command
		print_raw_to_user(u, (char*)JOIN_SUCCESS_CMD); 
		//Build string to send user. Different if joining room vs. creating new room
		sprintf(msg_buffer, "[Server]: Joining room '%s'\n \n", room_to_enter->room_name); 
		print_to_user(u, msg_buffer); //Send message to user
		printf("User %s joined existing room %s.\n", u->nick, room_to_enter->room_name); 
	}
	else {
		//Need to dynamically allocate name to store in chat_room object if a new room is created
		new_room_name = (char*)malloc(strlen(room_name) + 1); 
		strcpy(new_room_name, room_name); 

		room_to_enter = new_chat_room(new_room_name); //Create new chat room
		add_to_list(all_chat_rooms, room_to_enter); //Add new room to universal list of chat rooms
		add_to_list(room_to_enter->users_in_room, (void*)u); //Add user to new chat room's list of users

		remove_node(u->current_room->users_in_room, u); //Remove user from old chat room
		u->current_room = room_to_enter; //Set user's current room to the new room  

		//If we successfully added user to room, send success command
		print_raw_to_user(u, (char*)JOIN_SUCCESS_CMD); 
		//Build string to send user. Different if joining room vs. creating new room
		sprintf(msg_buffer, "[Server]: Creating new room '%s'\n \n", new_room_name); 
		print_to_user(u, msg_buffer); 
		printf("User %s joined new room %s.\n", u->nick, new_room_name); 
	}
}

void msg_all_users(char* msg, chat_room* room, char* sender) {
	char buffer[BUFFER_SIZE]; 
	int tmp_socket = -1; 
	node* tmp_node = room -> users_in_room -> head; 
	user* tmp_user = NULL; 
	int err = -1;

	while (tmp_node != NULL) {
		tmp_user = (user*)(tmp_node -> data); 
		tmp_socket = tmp_user -> user_socket;  

		sprintf(buffer, "%s %s says: %s\n", PRINT_CMD, sender, msg); 
		pthread_mutex_lock(&(tmp_user->user_sock_mutex)); 
		err = send(tmp_socket, buffer, strlen(buffer) + 1, 0); 
		pthread_mutex_unlock(&(tmp_user->user_sock_mutex)); 

		if (err < 0) {
			printf("Error sending msg %s to user %s\n", buffer, tmp_user -> nick); 
		}

		tmp_node = tmp_node -> next; 
	}
}

void list_users_in_room(user* u) { //Print out all the users in a user's room to the user

	node* tmp = u->current_room->users_in_room->head; 
	user* tmp_user = NULL;
	int user_no = 0; 
	char strbuffer[BUFFER_SIZE]; 

	print_to_user(u, "All users in room:"); 

	while (tmp != NULL) {
		tmp_user = (user*)(tmp->data);
		sprintf(strbuffer, ">User %d: %s", ++user_no, tmp_user->nick); 
		print_to_user(u, strbuffer); 
		tmp = tmp->next; 
	}

	print_to_user(u, " \n"); //Endline after list of users for pretty formatting. 

}	

void list_server_users(user* u) {
	node* tmp = all_users->head; 
	user* tmp_user = NULL;
	int user_no = 0; 
	char strbuffer[BUFFER_SIZE]; 

	print_to_user(u, "All users on server:"); 

	while (tmp != NULL) {
		tmp_user = (user*)(tmp->data);
		sprintf(strbuffer, ">User %d: %s in room %s", ++user_no, tmp_user->nick, tmp_user->current_room->room_name); 
		print_to_user(u, strbuffer); 
		tmp = tmp->next; 
	}

	print_to_user(u, " \n"); //Endline after list of users for pretty formatting. 
}

void list_all_rooms(user* u) {
	node* tmp = all_chat_rooms->head; 
	chat_room* tmp_room = NULL; 
	int room_no = 0; 
	char strbuffer[BUFFER_SIZE]; 

	print_to_user(u, "All chat rooms on server:"); 

	while (tmp != NULL) {
		tmp_room = (chat_room*)tmp->data; 
		sprintf(strbuffer, ">Room %d: %s", ++room_no, tmp_room->room_name); 
		print_to_user(u, strbuffer); 
		tmp = tmp->next; 	
	}

	print_to_user(u, " \n"); 
}


void print_to_user(user* u, char* msg) {

	char buffer[BUFFER_SIZE]; 
	sprintf(buffer, "%s %s", PRINT_CMD, msg); 
	int err = -1;

	pthread_mutex_lock(&(u->user_sock_mutex)); 
	err = send(u->user_socket, buffer, strlen(buffer) + 1, 0); 
	pthread_mutex_unlock(&(u->user_sock_mutex)); 

	if (err < 0) 
		printf("Error in 'print_to_user' function.\nError: %s\n", strerror(errno)); 
	else if (err == 0) {
		printf("Error in 'print_to_user': 0 bytes written.\nError: %s\n", strerror(errno)); 
	}
}

void print_raw_to_user(user* u, char* msg) {

	if (msg == NULL || strlen(msg) <=0) {
		printf("Error: NULL or 0-length message in print_raw_to_user()\n"); 
		return; 
	}

	int err = -1;

	pthread_mutex_lock(&(u->user_sock_mutex)); 
	err = send(u->user_socket, msg, strlen(msg) + 1, 0); 
	pthread_mutex_unlock(&(u->user_sock_mutex)); 

	if (err < 0) 
		printf("Error sending in 'print_raw_to_user' function.\nError: %s\n", strerror(errno)); 
	else if (err == 0) 
		printf("Error sending in 'print_raw_to_user': 0 bytes written.\nError: %s\n", strerror(errno)); 
	
}

chat_room* chat_room_exists(char* chat_room_name) {
	node* tmp = all_chat_rooms->head; 
	
	while (tmp != NULL) {
		if (strcasecmp(((chat_room*)(tmp->data))->room_name, chat_room_name)==0)
			return (chat_room*)(tmp->data); 
		tmp = tmp->next ;
	}
	return NULL; 
}
