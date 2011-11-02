#include "../include/server_mutexes.h"

void init_mutexes() {
	
	pthread_mutex_init(&sock_mutex, NULL); //Guarantee 2 threads don't simultaneously write to client socket
	pthread_mutex_init(&users_mutex, NULL); //Guarantee 2 threads don't both modify user list
	pthread_mutex_init(&chat_room_mutex, NULL); //Guarantee 2 threads don't both modify chat room list
}
