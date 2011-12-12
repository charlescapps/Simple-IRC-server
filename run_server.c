#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#include "include/users.h"
#include "include/start_server.h"
#include "include/misc.h"
#include "include/commands.h"
#include "include/constants.h"
#include "include/server_mutexes.h"
#include "include/global_server_cmds.h"

//As soon as we accept a client, we start a new thread so we can continue accepting clients. 
//We need this struct with the barebones information about a client so we can just call pthread_create with one arument
typedef struct user_info_s {
	pthread_t my_thread; 
	int my_socket; 
} user_info; 

void accept_loop(int welcome_socket); 
char* parse_connect_cmd(char* cmd, bool* quit); //Returns the nick for connect cmd
void wait_for_connect_cmd(user_info* client_socket); //A new thread just for the user is created as soon as the socket connects
void catch_broken_pipe(int signum); 

int main(int argc, char** argv) {

	int error_number; 

	if (argc != 2) {
		fprintf(stderr, "Error- need exactly 1 argument. Usage: run_server <socket_no>\n"); 
		exit(EXIT_FAILURE); 
	}


	//Get socket from args
	char* chosen_socket = argv[1]; 

	//Call method from start_server.h to get socket that accepts clients
	int socket_fd = start_server(chosen_socket); 

	//Call another function to start listening on this socket. 
	if ( (error_number = start_listening(socket_fd)) < 0) {
		fprintf(stderr, "Error listening on socket: %s\nExiting...\n", strerror(errno)); 
		exit(EXIT_FAILURE); 
	}

	//register function to deal with a broken pipe, e.g. when client exits program and server tries to write to stream
	//signal(SIGPIPE, catch_broken_pipe); 

	//Init mutexes used 
	init_mutexes(); 

	//Init list of users to empty list
	init_user_list(); 

	//Init list of chat rooms to empty list, instantiate lobby chat room
	init_chat_rooms(); 

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
	char partial_cmd[BUFFER_SIZE]; char to_send[BUFFER_SIZE];  
	char* next_cmd = NULL; char* nick = NULL; 
	bool err = false; bool quit = false; 
	int user_thread = ui->my_thread; 
	int new_socket_fd = ui->my_socket; 

	partial_cmd[0]='\0';	

	//While user inputs invalid connect command, keep reading commands from user
	while (true) {
		quit = false;

		if (next_cmd) 
			free(next_cmd); 

		next_cmd = get_next_cmd(new_socket_fd, partial_cmd, &err); 

		if (err) {
			printf("Error reading message from client. Terminating client socket %d\nError: %s\n", new_socket_fd, strerror(errno)); 
			close(new_socket_fd); 
			free(ui); 
			if (next_cmd) 
				free(next_cmd); 
			pthread_exit(NULL); 
		}
		else if (next_cmd == NULL) { //Havne't yet received a full command. 
			continue; 
		}
		else {
			printf("Received msg from client: '%s'\n", next_cmd); 
			trim_str(next_cmd); 
			nick = parse_connect_cmd(next_cmd, &quit); 
		}

		if (quit==true) {
			printf("Received +QUIT command from client. Terminating client socket %d.\n", new_socket_fd); 
			close(new_socket_fd); //close socket
			free(ui); 
			free(next_cmd); 
			pthread_exit(NULL); //free user
		}
		else if (nick == NULL) {
			printf("NULL nick received from client\n");
			write(new_socket_fd, FAIL_CMD, strlen(FAIL_CMD) + 1); 
		}
		else if (user_exists(nick)) {
			printf("Nick %s already exists.\n", nick);
			sprintf(to_send, "%s Nick already exists. Please try again with a different nick.\n", PRINT_CMD); 
			write(new_socket_fd, to_send, strlen(to_send) + 1); 
			free(nick); 
		}
		else if (!valid_charset(nick)) {
			printf("Nick %s not in valid charset.\n", nick); 
			sprintf(to_send, "%s Nick contains invalid chars. \n" 
							"Must be alphanumeric or '-', '_', '&', '*'.\n ", PRINT_CMD); 
			write(new_socket_fd, to_send, strlen(to_send) + 1); 
			free(nick); 
		}
		else {
			printf("New user '%s' entering lobby.\n", nick);  
			free(next_cmd); 
			init_user(nick, new_socket_fd, user_thread); 				
		}
	}
}

char* parse_connect_cmd(char* cmd, bool* quit) {
	
	*quit = false; 
	char* tok1 = strtok(cmd, " "); 
	char* tok2 = strtok(NULL, " "); 

	if (tok1 == NULL) {
		printf("Empty msg received from client.\n"); 
		return NULL; 
	}

	if (strcmp(tok1, QUIT_CMD) == 0) {
		*quit = true; 
		return NULL; 
	}

	if (strcmp(tok1, CONNECT_CMD) != 0 || tok2 == NULL) {
		return NULL; 
	}

	else {
		char* nick = (char*)malloc(sizeof(char)*(strlen(tok2) + 1)); 
		strcpy(nick, tok2); 
		return nick; 
	}

}

void catch_broken_pipe(int signum) {
	printf("Pipe broken trying to write to user's socket. Terminating user's thread.\n"); 
	pthread_exit(NULL); //Quit the thread for this user if a broken pipe occurs
}
