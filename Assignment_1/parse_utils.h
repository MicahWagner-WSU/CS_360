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

int get_count(int argc, char **argv);

FILE **get_files(int argc, char **argv, int count_present, int *num_of_files);

void free_files(FILE **files, int num_of_files);

int count_word_pairs(FILE **files, struct hash_table *table, int num_of_files);

int compare(const void *p, const void *q);

char *concat_string(char *str1, char *str2);

#endif
