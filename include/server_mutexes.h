#ifndef SERVER_MUTEX
#define SERVER_MUTEX

#include <pthread.h>

pthread_mutex_t sock_mutex; //Guarantee 2 threads don't simultaneously write to client socket
pthread_mutex_t users_mutex; //Guarantee 2 threads don't both modify user list
pthread_mutex_t chat_room_mutex; //Guarantee 2 threads don't both modify chat room list

//Must be called by server code before command loop
void init_mutexes(); 

#endif
