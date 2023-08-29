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

}

/*
	initializes a hash table, returns pointer to hash table
*/
struct hash_table *hash_init(unsigned int size) {

	// if size is 0, return null because 0 is an invalid size
	if(size == 0) return NULL;

	// allocate space for hash table on the heap
	struct hash_table *new_hash_table = malloc(sizeof(struct hash_table));

	// allocate space for hash entries
	new_hash_table->hash_entries = malloc(sizeof(struct hash_entry) * size);

	// set the current entry size in the corresponding hash table field
	new_hash_table->num_of_entries = size;

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

/*
	This funcion will first search if the specified key is in the hashmap
	if the key is in the hash map, we will return a void pointer to the data associated with the key
	if its not in the hash map, it will create a new key_value pair in the hashmap and return NULL

	So this function is used to get some data specified by a key, or create a new entry
*/
void *hash_put_if_absent(struct hash_table *table, char *key, void *value) {

	// check if we need to rehash, if so, rehash the hash table
	if(hash_check_for_rehash(table)) hash_rehash_table(table);

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
	this function will loop over each entry in the hash table and count total collisions
	total collisions represent the total amount of data in the hashmap
	once coutning all the data, we compare total_data/num_of_entries to the LOAD_FACTOR
	if its greater than the LOAD_FACTOR, the we need to rehash
*/
int hash_check_for_rehash(struct hash_table *table) {

	// reperesents total amount of data in the hashmap
	int total_data = 0;

	// loops over every entry and counts all data
	for(int i = 0; i < table->num_of_entries; i++) {

		// however many collisions this entry has is how much data is in the entry, so add that to total data
		total_data = total_data + table->hash_entries[i].collisions;

		// if we get larger than the LOAD_FACTOR, return 1 for true
		if(((double) total_data / table->num_of_entries) > LOAD_FACTOR) return 1;
	}

	// we dont need to rehash, return 0
	return 0;
}

/*
	this will rehash a given hash table, expanding the size by three
	when re-adding data to the new hashtable, we call hash_add_no_rehash since we know we dont need to check for rehashing
*/
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
	int total_data = hash_data_count(table);
	struct key_value *current;
	int new_kv_array_index = 0;

	struct key_value *new_kv_array = malloc(sizeof(struct key_value) * total_data);

	// loops over every entry and 
	for(int i = 0; i < table->num_of_entries; i++) {

		current = table->hash_entries[i].kv->next;

		while(current != NULL) {
			new_kv_array[new_kv_array_index].key = current->key;
			new_kv_array[new_kv_array_index].value = current->value;
			new_kv_array[new_kv_array_index].next = NULL;
			current = current->next;
			new_kv_array_index++;
		}
	}
	return new_kv_array;
}


int hash_data_count(struct hash_table *table) {

	int total_data = 0;

	// loops over every entry and counts all data
	for(int i = 0; i < table->num_of_entries; i++) {
		// however many collisions this entry has is how much data is in the entry, so add that to total data
		total_data = total_data + table->hash_entries[i].collisions;
	}

	return total_data;
}


/*
	This will completely free the hash table from the heap
*/
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


void list_combine(struct key_value *list_sentinel, struct key_value *list_sentinel_to_combine) {

}

/*
	This function frees each key value pair from a given list, deallocating the value and key
*/
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