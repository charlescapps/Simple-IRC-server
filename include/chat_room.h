#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include "users.h"
#include "list.h"
#include "commands.h"
#include "constants.h"
#include "misc.h"
#include "global_server_cmds.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/socket.h>

/*
 * The chat_room module manages the list of chat rooms. It also has functions for the loop to listen for 
 * commands from the client, and how to respond to these commands. 
 *
 * Forward declaration here so that gcc will compile properly. Could have also done 2 stages generating .o files
 * then linking, but this method always works. 
 */

typedef struct user_struct user; //Forward-declare typedef

struct chat_room_struct {
	
	list* users_in_room; //linked list of users in this chat room
	char* room_name; //Name of room
	char* current_msg; //Unused currently. History of msg previously sent to user
	pthread_mutex_t chat_room_mutex; //To avoid 2 threads editing user's room list simultaneously

};

typedef struct chat_room_struct chat_room; 

//Allocate LOBBY chat room and chat_room_list (see chat_room.c)
void init_chat_rooms(); 

//Allocate a single chat room. Initialize mutex, etc.
chat_room* new_chat_room(char* name); 

//Free a chat room. Destroys mutex, frees heap data, etc.
void free_chat_room(chat_room* cr); 

//Add user to the special LOBBY room
void add_user_to_lobby(user* u); 

//Adds user to room if it exists, otherwise creates new room and adds user
void add_user_to_room(user* u, char* room_name); 

//Receive-act on commands from client
void chat_room_loop(user* u);

//Act on a particular command received
void act_on_chat_cmd(char* msg, user* u); 

//Send message to all users in a chat room. Default behaviour of +MSG command
void msg_all_users(char* msg, chat_room* room, char* sender); 

//Send list of all users in a room to user u. +LIST command
void list_users_in_room(user* u); 

//Send list of all rooms on server to user u. +ROOMS command
void list_all_rooms(user* u); 

//Send list of all uesrs on server to user u. +LIST_ALL command.
void list_server_users(user* u); 

//Send a message to a single user. Prepends +MSG command
void print_to_user(user* u, char* msg); 

//Sends string msg to user. For sending commands other than +MSG
void print_raw_to_user(user* u, char* msg); 

//Returns chat room if it's in list, otherwise returns NULL
chat_room* chat_room_exists(char* chat_room_name); 

#endif
