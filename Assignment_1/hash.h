#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRC64_REV_POLY      0x95AC9329AC4BC9B5ULL
#define CRC64_INITIALIZER   0xFFFFFFFFFFFFFFFFULL
#define CRC64_TABLE_SIZE    256
#define INITIAL_HASH_SIZE	48 /* start as a multiple of three because we grow by a multiple of three */

struct hash_table {
	struct hash_entry *hash_entries;
	unsigned int num_of_entries;
};


struct hash_entry {
	struct key_value *kv;
	int collisions;
};

struct key_value {
	char *key;
	void *value;
	struct key_value *next;
};

struct hash_table *hash_init();

unsigned long long hash_index(char *);

//name inspired by java even though I dont like java
void *hash_put_if_absent(struct hash_table *table, char *key, void *value);

void check_for_rehash(struct hash_table *table);

void rehash_table(struct hash_table *table);

void rehash_list(struct key_value *list);

void hash_free(struct hash_table *table);

struct key_value *hash_to_array(struct hash_table *table);

struct key_value *list_init();

void list_add(struct key_value *list_sentinel, char *key, void *value);

void list_foreach(struct key_value *list, void (*function)(void *));

// data is stored from this function into data
void *list_get(struct key_value *list_sentinel, char *key);

void list_free(struct key_value *list_sentinel);

#endif