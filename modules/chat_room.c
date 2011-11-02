#include "../include/chat_room.h"

static const char* LOBBY_NAME = "LOBBY"; 
static list* all_chat_rooms = NULL; 
static chat_room* LOBBY = NULL; 

chat_room* new_chat_room(char* name) {

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

void *add_user_to_lobby(user* u) {
	int max_prompt_size = 100; 

	if (all_chat_rooms == NULL) {
		all_chat_rooms = new_list((void(*)(void*))free_chat_room); 
	}

	if (LOBBY == NULL) {
		LOBBY = new_chat_room((char*)LOBBY_NAME); 
		add_to_list(all_chat_rooms, (void*)LOBBY); 
	}

	char new_prompt[max_prompt_size]; 
	sprintf(new_prompt, "%s @ %s>", u->nick, LOBBY_NAME); 
	set_user_prompt(u, new_prompt); 

	add_to_list(LOBBY -> users_in_room, (void*)u); 
	u->current_room = LOBBY; 

	chat_room_loop(u); 

	return NULL; 
}

void chat_room_loop(user* u) {
	char buffer[BUFFER_SIZE]; 
	int err = -1; 

	while (true) {

		if ((err = read(u->user_socket, buffer, BUFFER_SIZE)) < 0) {
			printf("Error reading message from client.\nClosing socket #%d to client '%s'\nError: %s\n", u->user_socket, u->nick, strerror(errno)); 
			remove_user(u); 
			pthread_exit(NULL); 
		}	
		else if (err == 0) {
			printf("0 bytes read from client.\nClosing socket #%d to client '%s'\n", u->user_socket, u->nick); 
			remove_user(u); 
			pthread_exit(NULL); 
		}
		else {
			trim_str(buffer); 
			act_on_chat_cmd(buffer, u); 
		}

	}
}

void act_on_chat_cmd(char* cmd, user* u) {
	char* tok1 = NULL;
	//char* tok2 = NULL; 
	char* msg = NULL; 

	tok1 = strtok(cmd, " "); 

	if (strcmp(tok1, MSG_CMD) == 0) {
		msg = (char*)(&cmd[strlen(MSG_CMD)+1]); //Send the message as everything after +MSG
		printf("%s @ %s says: %s\n", u->nick, u->current_room->room_name, msg); 
		msg_all_users(msg, u->current_room, u->nick); 
	}
	else {
		printf("Received bad command from user '%s', sending fail msg\n", u->nick); 
		pthread_mutex_lock(&(u->user_sock_mutex)); 
		write(u->user_socket, FAIL_CMD, strlen(FAIL_CMD) + 1); 
		pthread_mutex_unlock(&(u->user_sock_mutex)); 
	}
}

void msg_all_users(char* msg, chat_room* room, char* sender) {
	char buffer[BUFFER_SIZE]; 
	int tmp_socket = -1; 
	node* tmp_node = room -> users_in_room -> head; 
	user* tmp_user = NULL; 

	while (tmp_node != NULL) {
		tmp_user = (user*)(tmp_node -> data); 
		tmp_socket = tmp_user -> user_socket;  

		sprintf(buffer, "%s %s says: %s\n", PRINT_CMD, sender, msg); 
		pthread_mutex_lock(&(tmp_user->user_sock_mutex)); 
		write(tmp_socket, buffer, strlen(buffer) + 1); 
		pthread_mutex_unlock(&(tmp_user->user_sock_mutex)); 

		tmp_node = tmp_node -> next; 
	}
}	
