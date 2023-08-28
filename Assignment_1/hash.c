#include "hash.h"

/*
	initializes a hash table, returns pointer to hash table
*/
struct hash_table *hash_init() {

	// allocate space for hash table on the heap
	struct hash_table *new_hash_table = malloc(sizeof(struct hash_table));

	// allocate space for hash entries, initially there are INITIAL_HASH_SIZE amount of entries
	new_hash_table->hash_entries = malloc(sizeof(struct hash_entry) * INITIAL_HASH_SIZE);

	// set the current entry size in the corresponding hash table field
	new_hash_table->num_of_entries = INITIAL_HASH_SIZE;

	// loop over each hash entry, and create a new list for each entry
	for(int i = 0; i < INITIAL_HASH_SIZE; i++) {
		// get a reference to the current hash entries
		struct hash_entry *current_entry = new_hash_table->hash_entries;

		// index to the current hash entry and initialize a list
		current_entry[i].kv = list_init();

		// set collisions to 0 since we haven't had any
		current_entry[i].collisions = 0;
	}

	// return hash table
	return new_hash_table;
}

/*
	This funcion will first search if the specified key is in the hashmap
	if the key is in the hash map, we will return a void pointer to the data associated with the key
	if its not in the hash map, it will create a new key_value pair in the hashmap and return NULL
*/
void *hash_put_if_absent(struct hash_table *table, char *key, void *value) {

	// get an index into the hashmap, mod with number of entries to remap hash_index(key) within the bounds of the table size
	unsigned long long table_index = hash_index(key) % table->num_of_entries;

	// get the entry associated with the key
	struct hash_entry current_entry = table->hash_entries[table_index];

	// see if key is already in list
	void *ret_val = list_get(current_entry.kv, key); 

	// if ret_val is NULL, then we need to add the data to the list
	if(ret_val == NULL) {

		// add the data to the list
		list_add(current_entry.kv, key, value);

		// we have a collision, so increment
		table->hash_entries[table_index].collisions++;

		// return NULL since we made a key value pair
		return NULL;
	} 

	// key value pair already exists, return a reference to the value
	return ret_val;

}

/*
	This will completely free the hash table from the heap
*/
void hash_free(struct hash_table *table) {

	// loop over each list from each hash entry
	for(int i = 0; i < INITIAL_HASH_SIZE; i++) {

		// free each list from each entry
		list_free(table->hash_entries[i].kv);
	}

	// free the array of hash entries
	free(table->hash_entries);

	// free the hash table 
	free(table);
}


/*
	given a string, this returns a very large number as an index into the hash table 
	the number returned is computed using voodoo maths
*/
unsigned long long hash_index(char *string) {
	static int initFlag = 0;
	static unsigned long long table[CRC64_TABLE_SIZE];
    
	if (!initFlag) { initFlag++;
		for (int i = 0; i < CRC64_TABLE_SIZE; i++) {
			unsigned long long part = i;
			for (int j = 0; j < 8; j++) {
				if (part & 1)
					part = (part >> 1) ^ CRC64_REV_POLY;
				else part >>= 1;
			}
			table[i] = part;
		}
	}
    
	unsigned long long crc = CRC64_INITIALIZER;
	while (*string)
		crc = table[(crc ^ *string++) & 0xff] ^ (crc >> 8);
	return crc;
}


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