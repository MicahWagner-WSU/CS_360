/*
Micah Wagner
CSE 360
Assignment 1

desc:
	This file contains all the functions for checking the input format and parsing the file
	this function also contains some handy function like compare and print_kv_array to print the final result

*/


#include "parse_utils.h"


/*
	This function takes in argc and argv, and returns the count variable if present
	if count is not present, we return 0
	if the user tried to specify count by using a "-"" and the format is incorrect, we return -1
*/
int get_count(int argc, char **argv) {

	int count;

	// this contains any extra characters that may have been included in the count argument 
	char extra[256] = {0};

	// this will try to parse the count argument, then try to find any extra characters
	int return_flag = sscanf(argv[1], "-%d%s ", &count, extra);

	// if return flag is 1, then we successfully parsed an integer only
	if(return_flag == 1) {

		// successfully parse count, return it
		return count;

	// user may have not specified count at all, check if user explicitly specified count incorrectly
	} else if(sscanf(argv[1], "-%s ", extra) == 1) {

		// user specified count wrong, return -1
		return -1;
	}

	// user didn't specify count vairable, return 0
	return 0;

}

/*
	takes in argc and argv and returns an array of file pointers
	count_present is simply used as a flag so we can avoid explicitly checking for it again
	this function also set *file_count = number of files specified on command line
	the reason we have file_count is so that we dont have to re-calculate it in other functions

	if we have an issue parsing the files, we will return NULL
*/
FILE **get_files(int argc, char **argv, int count_present, int *file_count) {

	// this gets the number of files
	// if count is present, the number of files is argc - 2, else its argc - 1
	int num_of_files = (count_present > 0) ? argc - 2 : argc - 1;

	// this is where we will start indexing into argv
	// if count is present, we start indexing at 2, else we start indexing at 1
	int file_argv_index = (count_present > 0) ? 2 : 1;
	int i = 0;

	// if we have no files return NULL
	if(num_of_files <= 0) return NULL;


	// allocate space for the array of file pointers
	FILE **files = malloc(sizeof(FILE) * num_of_files);

	// start looping over each file from argv
	while(i < num_of_files) {

		// attempt to open a file from argv
		files[i] = fopen(argv[file_argv_index], "r");

		// check if fopen failed, if it failed we want to deallocate all files and return NULL
		if(files[i] == NULL) {

			// fopen failed, we need to loop over the file pointer array and close all files
			// start looping at i - 1 because the current index is null
			while(--i >= 0) {

				// close the file
				fclose(files[i]);
			}

			// free file pointer array and return NULL since user put in a non existent file
			free(files);
			return NULL;
		}

		// successfully opened file, try to open the next one by incrementing argv index
		file_argv_index++;

		// increment index into file pointer array
		i++;
	}

	// successfully opened all files, set file count and return files
	*file_count = num_of_files;
	return files;
}


/*
	this frees all files from a file pointer array
*/
void free_files(FILE **files, int num_of_files) {

	// loop over all files in the file pointer array, size specified by num_of_files
	for(int i = 0; i < num_of_files; i++) {
		// close files at a certain index
		fclose(files[i]);
	}

	// free the file pointer array
	free(files);
}


/*
	this function will count the amount of consecutive pairs of words
	this function takes in the file pointer array, the table to hold the pairs of words,
	and the number of files to read from

*/
void count_word_pairs(FILE **files, struct hash_table *table, int num_of_files) {

	// prev is the previous word from a file relative to current
	char *prev;

	// current is the word ahead of prev from the same file 
	char *curr;

	// pair of words is the string "prev curr", with prev and curr being replace by there actual values
	char *pair_of_words;

	// start looping over each file
	for(int i = 0; i < num_of_files; i++) {

		// get the previous word from the file at current index i
		prev = getNextWord(files[i]);

		// get the current word from the file at the current index i
		curr = getNextWord(files[i]);

		// continue reading words from the current file until we reach the end of the file
		// curr will be NULL when we reach EOF
		while(curr != NULL) {

			// concatinate prev and current together
			pair_of_words = concat_string(prev, curr);

			// try to get the value using pair of words as our key
			int *value = (int *)hash_get(table, pair_of_words);

			// if we couldn't get the value, create a new key value pair in the hashmap
			if(value == NULL) {

				// allocate space for the data the hashmap will store
				int *data = malloc(sizeof(int));

				// set its initial value to 1 since this is the first time we've seen this pair of words
				*data = 1;

				// create a new key value pair in the hashmap using pair of words as a key and data as the value
				// cast the data pointer to a void pointer since our value is of type void *
				hash_add(table, pair_of_words, (void*) data);

				// free prev
				free(prev);

				// make prev now equal current
				prev = curr;

				// make curr equal the next word in the file
				curr = getNextWord(files[i]);

				// go back up to the while loop 
				continue;

			}

			// we already have found this pair of words, increment the value associated with the key
			*value = *value + 1;

			// free this pair_of_words since we dont need to create a new key value pair
			free(pair_of_words);

			// free prev
			free(prev);

			// make prev now equal current
			prev = curr;

			// make curr equal the next word in the file
			curr = getNextWord(files[i]);

		}

		// free prev, the while loop doesn't free the last one from the file
		free(prev);
	}

}

/*
	this function returns a new string allocated on the heap that concatinates str1 and str2
	more specifically, str1 and str2 get concatinated but with a space in between the words
*/
char *concat_string(char *str1, char *str2) {

	// allocate a new string with the size of both strings + 2
	// we add 2 to make room for the space in between str1 and str2, and the null terminator
	char *final_str = malloc(sizeof(char) * (strlen(str1) + strlen(str2) + 2));

	// initialize string indexes
	int i = 0; 
	int j = 0;

	// start looping over string 1 until we hit the null terminator
	while(str1[i]) {

		// set the current index of final_str equal to the current index of str1
		final_str[i] = str1[i];

		// inrement index into the strings
		i++;
	}

	// put in the space after string 1 and increment the index into final_str
	final_str[i++] = ' ';

	// start looping over string 2
	while(str2[j]) {
		// set the character at final_str equal to the current index into str 2
		// increment both the index into final_str and str2
		final_str[i++] = str2[j++];
	}

	// set the final byte to the nll terminator
	final_str[i] = '\0';

	// retrun the final string
	return final_str;
}

/* Reads characters from fd until a single word is assembled */
/* and returns a copy of the word allocated from the heap.   */
/* NULL is returned at EOF.									 */
/* Words are defined to be separated by whitespace and start */
/* with an alphabetic character.  All alphabetic characters  */
/* translated to lower case and punctuation is removed.      */

char* getNextWord(FILE* fd) {
	char ch;								/* holds current character */
	char wordBuffer[DICT_MAX_WORD_LEN];		/* buffer for build a word */
	int putChar = 0;						/* current pos in buffer   */

	assert(fd != NULL);		/* the file descriptor better not be NULL */

	/* read characters until we find an alphabetic one (or an EOF) */
	while ((ch = fgetc(fd)) != EOF) {
		if (isalpha(ch)) break;
	}
	if (ch == EOF) return NULL;		/* if we hit an EOF, we're done */

	/* otherwise, we have found the first character of the next word */
	wordBuffer[putChar++] = tolower(ch);

	/* loop, getting more characters (unless there's an EOF) */
	while ((ch = fgetc(fd)) != EOF) {
		/* the word is ended if we encounter whitespace */
		/* or if we run out of room in the buffer       */
		if (isspace(ch) || putChar >= DICT_MAX_WORD_LEN - 1) break;

		/* otherwise, copy the (lowercase) character into the word   */
		/* but only if it is alphanumeric, thus dropping punctuation */
		if (isalnum(ch)) {
			wordBuffer[putChar++] = tolower(ch);
		}
	}

	wordBuffer[putChar] = '\0';		/* terminate the word          */
	return strdup(wordBuffer);		/* re-allocate it off the heap */
}


/*
	prints an array of key_value pairs generated from the hashmap
	count will specify the top reacurring pairs of words to print up until count
	length is the amount of data in the hashmap
*/
int print_kv_array(struct key_value *kv_arr, int count, int length) {

	// if the length of the array is less than the count, or if length is 0, we have an error so return -1
	if(length < count || length == 0) return -1;

	//loop over the array up until count and print those key_value pairs
	for(int i = 0; i < (count > 0 ? count : length); i++) {

		// print the key value pairs
		printf("%10d %s\n", *(int *)kv_arr[i].value, kv_arr[i].key);
	}

	// return 1, everything was successfull
	return 1;
}

/*
	this will be used for the qsort function
	returns a number greater than 0 if the value at q > p
	returns a number less than 0 if the value at p > q
	its in this order because qsort will start sorting the greatest values first this way 
*/
int compare(const void *p, const void *q) {
	// get value at p and q (they are void pointers so we need to cast them)
    int l = *(int *)((struct key_value *)p)->value;
    int r = *(int *)((struct key_value *)q)->value;

    // return value at q minus value at p so that the array is arranged from greated to smallest
    return (r - l);
}



