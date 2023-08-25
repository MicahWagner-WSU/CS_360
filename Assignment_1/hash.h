#ifndef HASH_H
#define HASH_H

struct hash_entry {
	struct key_value *kv;
	int collisions;
	int num_of_items;
}

struct key_value {
	char *key;
	void *value;
	struct key_value *next;
}

struct hash_entry *hash_init();

int hash(char *);

void hash_add(struct hash_entry *table, char *key, void *value);

struct key_value *list_init();

void list_add(struct key_value *list, char *key, void *value);

#endif