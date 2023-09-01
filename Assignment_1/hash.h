#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRC64_REV_POLY      0x95AC9329AC4BC9B5ULL
#define CRC64_INITIALIZER   0xFFFFFFFFFFFFFFFFULL
#define CRC64_TABLE_SIZE    256

// start as a multiple of three because we grow by a multiple of three 
#define INITIAL_HASH_SIZE	15 

// if the amount of data in our hashmap is 75% of the number of entries in the hashmap, rehash
#define LOAD_FACTOR 0.75 
#define GROWTH_FACTOR 3


// this structure is the highest thing encapsulating the hash table
// contains a pointer to hash_entries (aka an array of hash_entries)
// also contains the number of entries, as well as the amount of data in the hash table
struct hash_table {
	struct hash_entry *hash_entries;
	unsigned int num_of_entries;
	unsigned int amount_of_data;
};


// this is the hash entry that the hashtable points to
// contains a pointer to key value pairs chained together
// also contains the amount of collisions we have for this entry
struct hash_entry {
	struct key_value *kv;
	int collisions;
};

// this is each key value pair within the hashmap
// contains the key, value, and a pointer to the next node in this chain
struct key_value {
	char *key;
	void *value;
	struct key_value *next;
};

/*
	initializes a hash table with the amount of entries specified by size 
	returns pointer to hash table
	returns NULL if any errors occur
*/
struct hash_table *hash_init(unsigned int size);

/*
	given a string, this returns a very large number as an index into the hash table 
	the number returned is computed using voodoo maths
*/
unsigned long long hash_index(char *);

/*
	this function adds a new key value pair to the given hash table
	it will use the given key and value specified by the user to create a new key_value pair
	this function does not check if the key_value pair is already in the hashmap
*/
int hash_add(struct hash_table *table, char *key, void *value);


/*
	this function checks the given hash table is the key specified is already in the hashmap
	if the return value is NULL, then their is no key_value pair for the specified key
	if it returns some pointer, that means there is a key_value pair specified by the key
*/
void *hash_get(struct hash_table *table, char *key);

/*
	this function will use total collisions and compare it to a load factor
	total collisions represent the total amount of data in the hashmap
	once retreiving amount of data, we compare total_data/num_of_entries to the LOAD_FACTOR
	if its greater than the LOAD_FACTOR, the we need to rehash
*/
int hash_check_for_rehash(struct hash_table *table);


/*
	this will rehash a given hash table, expanding the size by three
	when re-adding data to the new hashtable, we call hash_add_no_rehash since we know we dont need to check for rehashing
*/
void hash_rehash_table(struct hash_table *table);

/*
	This will completely free the hash table from the heap
	this also deallocates the key value pairs initialized by the user
*/
void hash_free(struct hash_table *table);

/*
	returns an array of key_value pairs that are currently stored in table
	this function will not dealocate anything from the hash map
	after freeing the hashmap, all the data in the returned array with point to invalid locations
	this returns NULL if an error occured
*/
struct key_value *hash_to_array(struct hash_table *table);

/*
	creates a list of key_value pairs
	the first index of this list is going to be a sentinel node, makes implementation easier
*/
struct key_value *list_init();

/*
	adds a new key value pair to the beginning of the list (after sentinel node)
	add them write after the sentinel in order to avoid looping over the whole list

	parameters are the list to add to (the sentinel), the asociated key and value of the new key_value pair
	returns NULL if an error occured
*/
void *list_add(struct key_value *list_sentinel, char *key, void *value);

/*
	given the sentinel node of a list and a key, this returns the value associated with that key
*/
void *list_get(struct key_value *list_sentinel, char *key);

/*
	This function frees each key value pair from a given list, deallocating the value and key
*/
void list_free(struct key_value *list_sentinel);

#endif