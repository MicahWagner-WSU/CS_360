/*
Micah Wagner
CSE 360
Assignment 1

desc:
	This file contains all the functions for checking the input format and parsing the file
	this file consists of ...

*/


#include "parse_utils.h"



int get_count(int argc, char **argv) {
	int count;
	char extra[256] = {0};

	int return_flag = sscanf(argv[1], "-%d%s ", &count, extra);

	if(return_flag == 1) {
		return count;
	} else if(sscanf(argv[1], "-%s ", extra) == 1) {
		return -1;
	}
	return 0;

}

FILE **get_files(int argc, char **argv, int count_present, int *file_count) {
	int num_of_files = (count_present > 0) ? argc - 2 : argc - 1;
	int file_argv_index = (count_present > 0) ? 2 : 1;
	int i = 0;

	if(num_of_files <= 0) return NULL;

	FILE **files = malloc(sizeof(FILE) * num_of_files);

	while(i < num_of_files) {
		files[i] = fopen(argv[file_argv_index], "r");
		if(files[i] == NULL) {
			while(--i >= 0) {
				fclose(files[i]);
			}

			free(files);
			return NULL;
		}
		file_argv_index++;
		i++;
	}

	*file_count = num_of_files;
	return files;
}

void free_files(FILE **files, int num_of_files) {

	for(int i = 0; i < num_of_files; i++) {
		fclose(files[i]);
	}

	free(files);
}

int count_word_pairs(FILE **files, struct hash_table *table, int num_of_files) {

	char *prev;
	char *curr;
	char *pair_of_words;

	for(int i = 0; i < num_of_files; i++) {

		prev = getNextWord(files[i]);
		curr = getNextWord(files[i]);

		while(curr != NULL) {
			pair_of_words = concat_string(prev, curr);

			int *value = (int *)hash_get(table, pair_of_words);

			if(value == NULL) {
				int *data = malloc(sizeof(int));
				*data = 1;

				hash_add(table, pair_of_words, (void*) data);
				free(prev);
				prev = curr;
				curr = getNextWord(files[i]);
				continue;

			}

			*value = *value + 1;
			free(pair_of_words);
			free(prev);
			prev = curr;
			curr = getNextWord(files[i]);

		}

		free(prev);
	}

}


char *concat_string(char *str1, char *str2) {
	char *final_str = malloc(sizeof(char) * (strlen(str1) + strlen(str2) + 2));
	int i = 0; 
	int j = 0;

	while(str1[i]) {
		final_str[i] = str1[i];
		i++;
	}

	final_str[i++] = ' ';

	while(str2[j]) {
		final_str[i++] = str2[j++];
	}

	final_str[i] = '\0';

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

int compare(const void *p, const void *q) {
    int l = *(int *)((struct key_value *)p)->value;
    int r = *(int *)((struct key_value *)q)->value;
    return (r - l);
}



