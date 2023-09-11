/*
Micah Wagner


name:
	lineNum -- see if a word is in the online dictionary
description:	
	See CS 360 IO lecture for details.
*/
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>



/*
Search for given word in dictionary specified by the path dictionaryName.
Return the line number if the word given was found, negative of the last line searched
if not found, or the error number if an error occurs.
*/

int lineNum(char *dictionaryName, char *word, int length) 
{
	char buffer[512] = {0};
	int store_errno;
	char *word_truncated = calloc(length + 1, sizeof(char));
	int dict_fd = open(dictionaryName, O_RDONLY);
	store_errno = errno;
	if (dict_fd == -1) {
		perror("Error opening file");
		free(word_truncated);
		return store_errno;
	}

	// high end of our binary seach space converted to line number
	int search_high = (lseek(dict_fd, 0, SEEK_END) / length);
	store_errno = errno;
	if (search_high == -1) {
		perror("Error lseeking");
		free(word_truncated);
		close(dict_fd);
		return errno;
	}

	//low end of binary search space
	int search_low = 1;
	int file_index;
	int line_number;

	// truncate the given word
	for(int i = 0; i < length - 1; i++){
		word_truncated[i] = word[i];
	}

	word_truncated[length] = '\0';


	// stop when low end of binary search space is larger than high end since word does not exist in file
	while (search_low <= search_high) {

		// check to see if we are getting an odd number, if so we want to round line number up
		if ((search_high + search_low) % 2 == 1) 
			line_number = ((search_high + search_low) / 2) + 1;
		else
			line_number = ((search_high + search_low) / 2);

		// set the file index to be half way in between the binary search space
		// we subtract by 1 because the first word sarts at lseek 0 (0 bytes in the file)
		file_index = lseek(dict_fd, (line_number - 1) * length, SEEK_SET);

		store_errno = errno;
		if (file_index == -1) {
			perror("Error lseeking");
			break;
		}

		if (read(dict_fd, buffer, length) == -1) {
			store_errno = errno;
			perror("Error reading");
			break;
		}

		int null_index = length - 1;
		// clean up the buffer by eliminating white space after the word
		do {
			if (buffer[--null_index] != ' ') {
				buffer[null_index + 1] = '\0';
				break;
			}
		} while (null_index);

		int comp = strcmp(buffer, word_truncated);

		// if comparison matches, return line number 
		if (comp == 0) {
			close(dict_fd);
			free(word_truncated);
			return line_number;
		} else if (comp > 0) {
			// word is alphabetically smaller, reduce search space by half
			// subtract one to get rid of the current word we're checking
			search_high = line_number - 1;
		} else if (comp < 0) {
			// word is alphabetically larger, reduce search space by half
			// add one to get rid of the current word we're checking
			search_low = line_number + 1;

		}
	} 

	free(word_truncated);

	close(dict_fd);

	// return line number * -1 because we didn't find the word
	return (store_errno == 0) ? line_number * -1: store_errno;
}