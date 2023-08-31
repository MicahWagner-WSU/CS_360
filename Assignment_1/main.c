/*
Micah Wagner
CSE 360
Assignment 1

desc:
	This is the main file that will call all the parse functions and print functions. 
	This file is relatively uneventful, it just calls all the functions from the other files.

*/
#include "parse_utils.h"


int main(int argc, char **argv) {

	if(argc <= 1) exit(1);

	int count = get_count(argc, argv);

	if(count == -1) exit(1);

	int num_of_files;
	FILE **files = get_files(argc, argv, count, &num_of_files);

	if(files == NULL) exit(1);

	struct hash_table *table = hash_init(INITIAL_HASH_SIZE);
	count_word_pairs(files, table, num_of_files);

	free_files(files, num_of_files);

	struct key_value *kv_array = hash_to_array(table);

	qsort((void *) kv_array, table->amount_of_data, sizeof(struct key_value), compare);

	//make a function for this taking count = 0 into account
	for(int i = 0; i < count; i++) {
		printf("%10d %s\n", *(int *)kv_array[i].value, kv_array[i].key);
	}

	hash_free(table);
	free(kv_array);


	return 0;
}