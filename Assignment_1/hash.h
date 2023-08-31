#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRC64_REV_POLY      0x95AC9329AC4BC9B5ULL
#define CRC64_INITIALIZER   0xFFFFFFFFFFFFFFFFULL
#define CRC64_TABLE_SIZE    256
#define INITIAL_HASH_SIZE	15 /* start as a multiple of three because we grow by a multiple of three */
#define LOAD_FACTOR 0.75
#define GROWTH_FACTOR 3

struct hash_table {
	struct hash_entry *hash_entries;
	unsigned int num_of_entries;
	unsigned int amount_of_data;
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

struct hash_table *hash_init(unsigned int size);

unsigned long long hash_index(char *);


//void *hash_put_if_absent(struct hash_table *table, char *key, void *value);

int hash_add(struct hash_table *table, char *key, void *value);

void *hash_get(struct hash_table *table, char *key);

int hash_check_for_rehash(struct hash_table *table);

void hash_rehash_table(struct hash_table *table);

void hash_rehash_list(struct key_value *list);

void hash_free(struct hash_table *table);


struct key_value *hash_to_array(struct hash_table *table);

struct key_value *list_init();

void *list_add(struct key_value *list_sentinel, char *key, void *value);

void *list_get(struct key_value *list_sentinel, char *key);

void list_free(struct key_value *list_sentinel);

#endif