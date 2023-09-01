/*
Micah Wagner
CSE 360
Assignment 1

desc:
	This is the main file that contains all the source functions to create and manipulate 
	a hashmap. This also contains several functions to create a manipulate a list, which the hash
	map function uses.

*/

#include "hash.h"



/*
	this function adds data to a hashmap without checking to rehash
	used in the rehash function when re-adding data to the hashmap 
	we know we dont need to check for rehashing since we just did that
*/
static void hash_add_no_rehash(struct hash_table *table, char *key, void *value) {

	// get an index into the hashmap, mod with number of entries to remap hash_index(key) within the bounds of the table size
	unsigned long long table_index = hash_index(key) % table->num_of_entries;

	// get the entry associated with the key
	struct hash_entry current_entry = table->hash_entries[table_index]; 

	// add the data to the list
	list_add(current_entry.kv, key, value);

	// we have a collision, so increment
	table->hash_entries[table_index].collisions++;

	// increment the amount of total data in the hashmap
	table->amount_of_data++;

}


struct hash_table *hash_init(unsigned int size) {

	// if size is 0, return null because 0 is an invalid size
	if(size == 0) return NULL;

	// allocate space for hash table on the heap
	struct hash_table *new_hash_table = malloc(sizeof(struct hash_table));

	//malloc failed, return null
	if(new_hash_table == NULL) return NULL;

	// allocate space for hash entries
	new_hash_table->hash_entries = malloc(sizeof(struct hash_entry) * size);

	// malloc failed, return null
	if(new_hash_table->hash_entries == NULL) return NULL;

	// set the current entry size in the corresponding hash table field, make amount of data 0 because this is a new hashmap
	new_hash_table->num_of_entries = size;
	new_hash_table->amount_of_data = 0;

	// loop over each hash entry, and create a new list for each entry
	for(int i = 0; i < size; i++) {
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


int hash_add(struct hash_table *table, char *key, void *value) {

	// get an index into the hashmap, mod with number of entries to remap hash_index(key) within the bounds of the table size
	unsigned long long table_index = hash_index(key) % table->num_of_entries;

	// add the data to the list
	void *ret_val = list_add(table->hash_entries[table_index].kv, key, value);

	// we have a collision, so increment
	table->hash_entries[table_index].collisions++;

	// more data added to table, increment data counter
	table->amount_of_data++;

	// check if we need to rehash, if so, rehash the hash table
	if(hash_check_for_rehash(table)) hash_rehash_table(table);

	if(ret_val == NULL) return -1;

	return 1;
}


void *hash_get(struct hash_table *table, char *key) {
	// get an index into the hashmap, mod with number of entries to remap hash_index(key) within the bounds of the table size
	unsigned long long table_index = hash_index(key) % table->num_of_entries;

	// see if key is already in list
	void *ret_val = list_get(table->hash_entries[table_index].kv, key); 

	// return a reference to the value
	return ret_val;
}


int hash_check_for_rehash(struct hash_table *table) {

	// reperesents total amount of data in the hashmap
	int total_data = table->amount_of_data;


	// if we get larger than the LOAD_FACTOR, return 1 for true
	if(((double) total_data / table->num_of_entries) > LOAD_FACTOR) return 1;


	// we dont need to rehash, return 0
	return 0;
}


void hash_rehash_table(struct hash_table *table) {

	// create a temperary new hashmap 3 times the size as the original
	struct hash_table *tmp = hash_init(table->num_of_entries * GROWTH_FACTOR);

	// current and previous to point to certain indexes in each chain
	struct key_value *current;
	struct key_value *prev;

	// loop over each list from each hash entry
	for(int i = 0; i < table->num_of_entries; i++) {

		// prev points to the key value pair before current
		prev = table->hash_entries[i].kv;

		// points to key value pair after prev
		current = table->hash_entries[i].kv->next;

		// loop over each key value pair within the list, quite if current points to NULL
		while(current != NULL) {

			// add the key value pair to the new temperary hashmap
			// since the sentinel has NULL keys and values we have to free prev before we increment, then free prev outside the loop 
			hash_add_no_rehash(tmp, current->key, current->value);

			// free the previous node, notice that we dont dealocate the key and value
			free(prev);

			// increment the previous pointer, and increment the current pointer
			prev = current;
			current = current->next;

		}

		// free the previous node since the while loop quites before we free that one
		free(prev);

	}

	// free the array of hash entries
	free(table->hash_entries);

	// set the table's hash_entries equal to the temperary one
	table->hash_entries = tmp->hash_entries;

	// update the tables size
	table->num_of_entries = tmp->num_of_entries;

	// free the temperary hashmap
	free(tmp);

}


struct key_value *hash_to_array(struct hash_table *table) {

	// get total amount of data in the hashmap
	int total_data = table->amount_of_data;

	// index into each linked list from the hashmap
	struct key_value *current;

	// index into the new array we will return
	int new_kv_array_index = 0;

	// initialize a new array on the heap big enough to contain all data in the hashmap
	struct key_value *new_kv_array = malloc(sizeof(struct key_value) * total_data);

	// return NULL, malloc failed
	if (new_kv_array == NULL) return NULL;

	// loops over every entry and copy data from each link list to the array 
	for(int i = 0; i < table->num_of_entries; i++) {

		// set surrent to point to the current entries linked list
		current = table->hash_entries[i].kv->next;

		// loop over each key value pair in the linked list, quit if we hit null as thats the last pair
		while(current != NULL) {

			// copy over the data to the new array of key value pairs
			new_kv_array[new_kv_array_index].key = current->key;
			new_kv_array[new_kv_array_index].value = current->value;

			// set next to null because we dont care about mainting the linked list structure
			new_kv_array[new_kv_array_index].next = NULL;

			//increment index into linked list and index into array
			current = current->next;
			new_kv_array_index++;
		}
	}

	// return the new array of key value pairs
	return new_kv_array;
}



void hash_free(struct hash_table *table) {

	// loop over each list from each hash entry
	for(int i = 0; i < table->num_of_entries; i++) {

		// free each list from each entry
		list_free(table->hash_entries[i].kv);
	}

	// free the array of hash entries
	free(table->hash_entries);

	// free the hash table 
	free(table);
}


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


struct key_value *list_init() {

	// create a pointer to a new list, allocate space on heap for sentinel node
	struct key_value *new_list = malloc(sizeof(struct key_value));	

	//malloc failed, return null
	if(new_list == NULL) return NULL;

	// initialize all data on sentinel node to null
	new_list->key = NULL;
	new_list->value = NULL;
	new_list->next = NULL;

	// return sentinel node, acts as first index into list
	return new_list;
}

void *list_add(struct key_value *list_sentinel, char *key, void *value) {

	// create the new key_value pair on the heap, and set its key and value pairs accordingly
	struct key_value *new_key_val = malloc(sizeof(struct key_value));

	//malloc failed, return null
	if(new_key_val == NULL) return NULL;

	new_key_val->key = key;
	new_key_val->value = value;

	// the new key value pair points to what the sentinel used to point to
	new_key_val->next = list_sentinel->next;

	// sentinel now points to new key value pair, every pair added is inserted infront of sentinel
	list_sentinel->next = new_key_val;
}


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

void list_free(struct key_value *list_sentinel) {

	// points to most current node 
	struct key_value *current = list_sentinel;

	// points to previous node with respect to the current node (since first node is sentinel, this points to null)
	struct key_value *prev = NULL;

	//loops over each node until current points to null (we've reached the end)
	while(current != NULL) {

		// prev now points to the current node
		prev = current;

		// increment to current node to point to the next node
		current = current->next;

		// free the key, value, and prev key_value struct itself
		free(prev->key);
		free(prev->value);
		free(prev);
	}

}