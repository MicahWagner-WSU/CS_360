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

	// if no arguments specified, exit
	if(argc <= 1) exit(1);

	// get the count specified by user
	int count = get_count(argc, argv);

	// user didn't specify count correctly, exit
	if(count == -1) exit(1);

	// get the number of files and get an array of all the files user specified to open
	int num_of_files;
	FILE **files = get_files(argc, argv, count, &num_of_files);

	// user specified non existent file, exit
	if(files == NULL) exit(1);

	// create a hash table of INITIAL_HASH_SIZE
	struct hash_table *table = hash_init(INITIAL_HASH_SIZE);

	// start counting pairs of words from the files given from the user using the hashmap
	count_word_pairs(files, table, num_of_files);

	// free the files the user gave us
	free_files(files, num_of_files);

	// create a key value array from the data in the hashmap
	struct key_value *kv_array = hash_to_array(table);

	// sort the array using qsort so we can print the array quickly
	qsort((void *) kv_array, table->amount_of_data, sizeof(struct key_value), compare);

	// print the array up to a specified count from the user
	int return_flag = print_kv_array(kv_array, count, table->amount_of_data);

	// if the return flag was -1, user specified invalid count size or a file had nothing in it
	if(return_flag == -1) {
		// free the hashmap and key value array, exit 
		hash_free(table);
		free(kv_array);
		exit(1);
	}

	// free the hashmap and key value array
	hash_free(table);
	free(kv_array);


	return 0;
}