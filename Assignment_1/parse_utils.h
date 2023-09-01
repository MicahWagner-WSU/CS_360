#ifndef GETWORD_H
#define GETWORD_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "hash.h"

#define DICT_MAX_WORD_LEN	256		/* maximum length of a word (+1) */

/* Reads characters from fd until a single word is assembled */
/* and returns a copy of the word allocated from the heap.   */
/* NULL is returned at EOF.                                  */

/* Words are defined to be separated by whitespace and start */
/* with an alphabetic character.  All alphabetic characters  */
/* translated to lower case and punctuation is removed.      */

char* getNextWord(FILE* fd);

/*
	This function takes in argc and argv, and returns the count variable if present
	if count is not present, we return 0
	if the user tried to specify count by using a "-"" and the format is incorrect, we return -1
*/
int get_count(int argc, char **argv);

/*
	takes in argc and argv and returns an array of file pointers
	count_present is simply used as a flag so we can avoid explicitly checking for it again
	this function also set *file_count = number of files specified on command line
	the reason we have file_count is so that we dont have to re-calculate it in other functions

	if we have an issue parsing the files, we will return NULL
*/
FILE **get_files(int argc, char **argv, int count_present, int *num_of_files);

/*
	this frees all files from a file pointer array
	the number of files in the array is needed in order to free all files in the array
*/
void free_files(FILE **files, int num_of_files);

/*
	this function will count the amount of consecutive pairs of words
	this function takes in the file pointer array, the table to hold the pairs of words,
	and the number of files to read from

*/
void count_word_pairs(FILE **files, struct hash_table *table, int num_of_files);

/*
	this will be used for the qsort function
	returns a number greater than 0 if the value at q > p
	returns a number less than 0 if the value at p > q
	its in this order because qsort will start sorting the greatest values first this way 
*/
int compare(const void *p, const void *q);

/*
	prints an array of key_value pairs generated from the hashmap
	count will specify the top reacurring pairs of words to print up until count
	length is the amount of data in the hashmap
*/
int print_kv_array(struct key_value *kv_arr, int count, int length);

/*
	this function returns a new string allocated on the heap that concatinates str1 and str2
	more specifically, str1 and str2 get concatinated but with a space in between the words
*/
char *concat_string(char *str1, char *str2);

#endif
