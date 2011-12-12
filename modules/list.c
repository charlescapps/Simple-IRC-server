#include "../include/list.h"

node* new_node(void* data) { //Allocate a new list node
	node* n = (node*)malloc(sizeof(node)); 
	n->data = data; 
	n->next = NULL; 
	return n; 
}

list* new_list(void (*free_fnc)(void*)) {//Allocate a new list; free_fnc is a function pointer to a fucntion for freeing the data in the list
	list* l = (list*) malloc(sizeof(list)); 
	l->free_data = free_fnc; 
	l->head = NULL; 
	return l; 
}

void add_to_list(list* l, void* data) { //Adds node to list and returns the new head
	node* new_head = new_node(data); 
	new_head -> next = l->head; 
	l->head = new_head; 
}

void remove_front(list* l) { //Removes a node from the front. Doesn't free data, because I need to manage it for this project
	if (l->head == NULL) {
		return; 
	}

	node* old_head = l->head; 
	l->head = old_head->next; 
	free(old_head); //Call function to free the old head
}

void remove_node(list* l, void* data){ //Removes node from list without freeing data
	node* iter = l->head; 
	node* to_delete; 
	if (!iter) //empty list
		return; 

	if (iter->data == data) { //Data in first node, just remove the front
		remove_front(l); 
		return; 
	}

	while (iter->next && iter->next->data != data) {
		iter = iter->next;
	}

	//Never found node to remove
	if (iter->next==NULL)
		return; 

	//Found node to remove; remove it and free the node
	to_delete = iter->next; 
	iter->next = iter->next->next; 

	free(to_delete); 

}

int list_size(list* l) { //Count number of nodes in a list

	if (l == NULL) {
		return 0; 
	}

	node* tmp = l->head; 
	int cnt = 0; 

	while (tmp != NULL) {
		cnt++; 
		tmp = tmp->next; 
	}

	return cnt; 
}

bool is_empty(list* l) {
	if (l->head == NULL)
		return true; 
	return false; 
}
