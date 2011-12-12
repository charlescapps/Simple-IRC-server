#include "../include/start_server.h"

static const int MAX_PENDING_CONNECTIONS = 256; 

int start_server(char* socket_str) {
	struct addrinfo hints; 
	struct addrinfo *result, *rp; //result stores addresses returned by getaddrinfo; rp is just a pointer to iterate  
	int socket_fd, error_code; 

	memset(&hints, 0, sizeof(struct addrinfo)); //Set hints to default
	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_STREAM; //Two-way reliable communication
	hints.ai_flags = AI_PASSIVE; //Flag for server
	hints.ai_protocol = 0; //Any protocol. Will choose IPv4 due to SOCK_STREAM choice 
	hints.ai_canonname = NULL; //According to docs, these should be NULL for binding an accept socket 
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	error_code = getaddrinfo(NULL, socket_str, &hints, &result); //Get an address that can be used in bind to host server
	
	if (error_code != 0) {
		fprintf(stderr, "Error calling getaddrinfo: %s\nExiting...\n", gai_strerror(error_code)); 
		exit(EXIT_FAILURE); 
	}

	for (rp = result; rp!=NULL; rp = rp->ai_next) { //Try all sockets returned in linked list
		socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); 
		//printf("Socket FD obtained: %d\n", socket_fd); 

		if (socket_fd < 0)  //Failed to open socket, try next one
			continue; 

		if (bind(socket_fd, rp->ai_addr, rp->ai_addrlen) == 0) //Succeeded, use this socket
			break; 

		fprintf(stderr, "Error calling bind: %s\n", strerror(errno)); 

		close(socket_fd); //If we got a socket, but failed to bind, close it an try the next one
	}

	freeaddrinfo(result); //Result already parsed, free memory

	if (rp == NULL) {
		fprintf(stderr, "Error: Failed to bind a socket!\nExiting...\n"); 
		exit(EXIT_FAILURE); 
	}

	printf("Accepting clients on socket %s. File descriptor: %d\n", socket_str, socket_fd);  

	return socket_fd; 
}	

int start_listening(int socket_fd) {

	return listen(socket_fd, MAX_PENDING_CONNECTIONS); 
	
}
