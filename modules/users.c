#include "../include/users.h"

pthread_mutex_t sock_mutex; //guarantees 2 threads don't write to 

void init_user_list() {//Create empty list of users if it hasn't been initialized
	all_users = new_list((void(*)(void*))free_user); 
}

void init_user(char* nick, int sock, pthread_t user_thread) {

	user* new_user = (user*)malloc(sizeof(user)); 
	new_user->user_socket = sock; 
	new_user->current_room = NULL;
	new_user->nick = nick;
	new_user->user_thread = user_thread; 
	pthread_mutex_init(&(new_user->user_sock_mutex), NULL); 

	pthread_mutex_lock(&users_mutex); //Atomically add user to list  
	add_to_list(all_users, new_user); 
	pthread_mutex_unlock(&users_mutex); 

	add_user_to_lobby(new_user);  
}

void free_user(user* u) {
	close(u->user_socket); 
	pthread_mutex_destroy(&(u->user_sock_mutex)); 
	free(u->nick); 
	free(u); 
}

void remove_user(user* u) { //removes user from all_users list, current chat room, then frees user
	if (u->current_room != NULL) {//Remove user from its current chat room
		pthread_mutex_lock(&(u->current_room->chat_room_mutex)); 
		remove_node(u->current_room->users_in_room, (void*)u); 
		pthread_mutex_unlock(&(u->current_room->chat_room_mutex)); 
	}

	pthread_mutex_lock(&users_mutex); 
	remove_node(all_users, (void*)u); //Remove user from global list of users
	pthread_mutex_unlock(&users_mutex); 

	free_user(u); //Finally, close user socket, destroy thread, and free memory
}

void set_user_prompt(user* u, char* new_prompt) {

	char msg_to_client[BUFFER_SIZE]; 
	int bytes_written;
	
	sprintf(msg_to_client, "%s %s", SET_PROMPT_CMD, new_prompt); 

	pthread_mutex_lock(&(u->user_sock_mutex)); //atomically write to user's socket using user's mutex
	bytes_written = send(u->user_socket, msg_to_client, strlen(msg_to_client) + 1, 0);
	pthread_mutex_unlock(&(u->user_sock_mutex)); 

	if (bytes_written < 0) {
		printf("Error sending new prompt.\nError: %s\n", strerror(errno)); 
	}
	else if (bytes_written == 0) {
		printf("Error sending new prompt: 0 bytes written.\nError: %s\n", strerror(errno)); 
	}
}

bool user_exists(char* nick) {

	user* tmp_u; 
	bool exists = false; 
	node* iter = all_users->head;

	while (iter != NULL) {
		tmp_u = (user*)iter->data;
		if (strcasecmp(tmp_u->nick, nick)==0) { 
			exists = true; 
			break;
		}
		iter=iter->next;
	}	
	return exists; 
}
