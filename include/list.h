#ifndef _LIST
#define _LIST

#include <stdlib.h>

typedef struct node_struct node; //Forward declaration so a list can have a next pointer

typedef struct node_struct { //Node type. Holds arbitrary data

	void* data; 
	node* next; 

} node_str; 

typedef struct { //List type

	node* head; 

	void (*free_data)(void*); //Function that frees the data in node. 

} list;

node* new_node(void* data); //Allocate a new list node

list* new_list(void (*free_fnc)(void*)); //Allocate a new empty list. Pass in function pointer for freeing data in list

void add_to_list(list* l, void* data); //Adds node to list and returns the new head

void remove_front(list* l); //Removes a node from the front and frees the data

int list_size(list* l); //Count number of nodes in a list

void remove_node(list* l, void* data); //Removes node from list without freeing data

#endif
