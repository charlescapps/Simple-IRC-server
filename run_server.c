#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "stdbool.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netdb.h"
#include "errno.h"
#include "signal.h"

#include "include/users.h"
#include "include/start_server.h"
#include "include/my_funcs.h"
#include "include/commands.h"
#include "include/server_mutexes.h"

//As soon as we accept a client, we start a new thread so we can continue accepting clients. 
//We need this struct with the barebones information about a client so we can just call pthread_create with one arument
typedef struct user_info_s {
	pthread_t my_thread; 
	int my_socket; 
} user_info; 

const char* CONNECT_MSG = "+PRINT Connected as user '%s'.\nNow entering lobby.\n"; 

void accept_loop(int welcome_socket); 
char* parse_connect_cmd(char* cmd, bool* quit); //Returns the nick for connect cmd
void wait_for_connect_cmd(user_info* client_socket); //A new thread just for the user is created as soon as the socket connects
void catch_broken_pipe(int signum); 

int main(int argc, char** argv) {

	int error_number; 

	if (argc != 2) {
		fprintf(stderr, "Error- need exactly 1 argument. Usage: test_server <socket_no>\n"); 
		exit(EXIT_FAILURE); 
	}


	//Get socket from args
	char* chosen_socket = argv[1]; 

	//Call method from start_server.h to get socket that accepts clients
	int socket_fd = start_server(chosen_socket); 

	//Call another function to start listening on this socket. 
	if ( (error_number = start_listening(socket_fd)) < 0) {
		fprintf(stderr, "Error listening on socket: %s\n", strerror(errno)); 
		exit(EXIT_FAILURE); 
	}

	//register function to deal with a broken pipe, e.g. when client exits program and server tries to write to stream
	signal(SIGPIPE, catch_broken_pipe); 

	//Init mutexes used 
	init_mutexes(); 

	//Init list of users to empty list
	init_user_list(); 

	//Start loop to accept clients
	accept_loop(socket_fd); 

	return EXIT_SUCCESS;
}

void accept_loop(int server_socket) {

	int new_socket_fd = -1; //File descriptor of connecting client
	socklen_t client_length = sizeof(struct sockaddr); 
	struct sockaddr client_addr; 
	user_info* ui; 

	//reset client address object
	memset(&client_addr, 0, sizeof(struct sockaddr)); 

	while (true) {
		//Wait for client to connect
		new_socket_fd = accept(server_socket, &client_addr, &client_length); 

		if (new_socket_fd < 0) {
			printf("Error accepting client connection: %s", strerror(errno)); 
			continue; 
		}

		ui = (user_info*)malloc(sizeof(user_info)); 
		ui -> my_socket = new_socket_fd; 

		pthread_create(&(ui->my_thread), NULL, (void*(*)(void*))wait_for_connect_cmd, (void*)(ui)); 

	}
}

//Wait until user types +CONNECT <nick> to enter the lobby
void wait_for_connect_cmd(user_info* ui) {
	char buffer[BUFFER_SIZE]; 
	int user_thread = ui->my_thread; 
	int new_socket_fd = ui->my_socket; 
	char* nick = NULL; 
	bool quit = false; 

	//While user inputs invalid connect command, try to read again
	while (nick == NULL) {
		if (read(new_socket_fd, buffer, BUFFER_SIZE) < 0) {
			printf("Error reading message from client\n"); 
			continue; 
		}	
		else {
			printf("Received from client: %s\n", buffer); 
			trim_str(buffer); 
			nick = parse_connect_cmd(buffer, &quit); 
		}
		if (quit==true) {
			printf("Received +QUIT command from client. Terminating thread for client socket %d.\n", new_socket_fd); 
			close(new_socket_fd); 
			pthread_exit(NULL); 
		}
		if (nick == NULL) {
			printf("Nick from client is NULL...sending fail msg.\n");
			write(new_socket_fd, FAIL_CMD, strlen(FAIL_CMD) + 1); 
			continue; 	
		}
		else {
			printf("New user '%s' entering lobby.\n", nick);  
			sprintf(buffer, CONNECT_MSG, nick);
			write(new_socket_fd, buffer, strlen(buffer) + 1); 
			init_user(nick, new_socket_fd, user_thread); 				
		}
	}
}

char* parse_connect_cmd(char* cmd, bool* quit) {
	
	*quit = false; 
	char* tok1 = strtok(cmd, " "); 
	char* tok2 = strtok(NULL, " "); 

	if (tok1 == NULL || tok2 == NULL) {
		printf("Null msg received from client\n"); 
		return NULL; 
	}

	if (strcmp(tok1, QUIT_CMD) == 0) {
		*quit = true; 
		return NULL; 
	}

	if (strcmp(tok1, CONNECT_CMD) != 0) {
		printf("Invalid connect command from client without nick: %s\n", tok1); 
		return NULL; 
	}

	else {
		char* nick = (char*)malloc(sizeof(char)*(strlen(tok2) + 1)); 
		strcpy(nick, tok2); 
		return nick; 
	}

}

void catch_broken_pipe(int signum) {
	printf("Pipe broken trying to read from user's socket. Terminating user's thread.\n"); 
	pthread_exit(NULL); //Quit the thread for this user if a broken pipe occurs
}
