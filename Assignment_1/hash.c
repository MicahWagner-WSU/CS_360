#include "hash.h"


/*
	creates a list of key_value pairs
	the first index of this list is going to be a sentinel node, makes implementation easier
*/
struct key_value *list_init() {

	// create a pointer to a new list, allocate space on heap for sentinel node
	struct key_value *new_list = malloc(sizeof(struct key_value));	/* ask about if we should check for error here, and how so*/

	// initialize all data on sentinel node to null
	new_list->key = NULL;
	new_list->value = NULL;
	new_list->next = NULL;

	// return sentinel node, acts as first index into list
	return new_list;
}

/*
	adds a new key value pair to the beginning of the list (after sentinel node)
	add them write after the sentinel in order to avoid looping over the whole list

	parameters are the list to add to (the sentinel), the asociated key and value of the new key_value pair
*/
void list_add(struct key_value *list_sentinel, char *key, void *value) {

	// create the new key_value pair on the heap, and set its key and value pairs accordingly
	struct key_value *new_key_val = malloc(sizeof(struct key_value));
	new_key_val->key = key;
	new_key_val->value = value;

	// the new key value pair points to what the sentinel used to point to
	new_key_val->next = list_sentinel->next;

	// sentinel now points to new key value pair, every pair added is inserted infront of sentinel
	list_sentinel->next = new_key_val;
}

void list_foreach(struct key_value *list, void(*function)(void *)) {

}

/*
	given the sentinel node of a list and a key, this returns the value associated with that key
*/
void *list_get(struct key_value *list_sentinel, char *key)	{

	// create an index into the list, start pointing at true first key value pair
	struct key_value *current = list_sentinel->next;

	//loops over each node until current points to null (we've reached the end)
	while(current != NULL) {

		// compare the index key with the given key, if equal, return the value
		if(strcmp(current->key, key) == 0) return current->value;

		// keys weren't equal, increment index
		current = current->next;
	}

	// value was not found, return NULL
	return NULL;
}

/*
	This function frees each key value pair from a given list, deallocating the value and key
*/
void list_free(struct key_value *list_sentinel) {

	// points to most current node 
	struct key_value *current = list_sentinel->next;

	// points to previous node with respect to the current node
	struct key_value *prev = list_sentinel;

	//loops over each node until current points to null (we've reached the end)
	while(current != NULL) {

		// free the key, value, and prev key_value struct itself
		free(prev->key);
		free(prev->value);
		free(prev);

		// prev now points to the current node
		prev = current;

		// increment to current node to point to the next node
		current = current->next;
	}

	// free the last previous node, the while loop deosn't cover this one
	free(prev->key);
	free(prev->value);
	free(prev);
}