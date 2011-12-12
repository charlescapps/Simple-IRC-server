#ifndef USERS_H
#define USERS_H

#include "chat_room.h"
#include "list.h"
#include "commands.h"
#include "server_mutexes.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

/*
 * The users module manages creating new users, freeing users, removing users from all lists, manages
 * the global user list all_users. 
 *
 * Also processes some commands such as set_user_prompt (for +PRINT) and checking if a user is in the global list. 
 */

//Forward-declare this struct so that linking works. 
typedef struct chat_room_struct chat_room;

struct user_struct {
	char* nick; 
	pthread_t user_thread; //Each user has its own thread
	int user_socket; 
	chat_room* current_room; 
	pthread_mutex_t user_sock_mutex; 
}; 

typedef struct user_struct user; 

list* all_users; //List of all users. 

void init_user_list(); //create empty list of users

void init_user(char* nick, int sock, pthread_t user_thread); //Creates a new user object and adds it to the list all_users
//Also adds user to LOBBY room

void set_user_prompt(user* u, char* new_prompt); //+PROMPT command

void free_user(user* u); //Free data associated with a user u

void remove_user(user* u); //removes user from all_users list, current chat room, then frees user

bool user_exists(char* nick); //Checks if user is in global list of all users

#endif
