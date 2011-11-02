#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include "users.h"
#include "list.h"
#include "commands.h"
#include "my_funcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

/*
 * I never remember exactly how to do these forward declarations, and I have no idea why they were necessary. 
 * The user struct referred to a chat_room but NOT vice-versa. 
 * So...presumedly when I included chat_room.h, users.h would have that type available. 
 * BUT, for some bloody reason it doesn't work in C, so I have to use forward declarations like this. 
 *
 * Must have something to do with the fact that functions in the chat_rooms.h file use the "user" type, and functions in the users.h file use the "chat_room" type. 
 */

struct user_struct; 
typedef struct user_struct user; 

struct chat_room_struct {
	
	list* users_in_room; //linked list of users in this chat room
	char* room_name; //Name of room
	char* current_msg; //Not sure if I need this, had some reason
	pthread_mutex_t chat_room_mutex; //To avoid 2 threads editing user list simultaneously

};

typedef struct chat_room_struct chat_room; 


chat_room* new_chat_room(char* name); 
void free_chat_room(chat_room* cr); 

void *add_user_to_lobby(user* u); 

void chat_room_loop(user* u);

void act_on_chat_cmd(char* msg, user* u); 

void msg_all_users(char* msg, chat_room* room, char* sender); 

#endif
