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



/*
Search for given word in dictionary using file descriptor fd.
Return the line number if the word was found, negative of the last line searched
if not found, or the error number if an error occurs.
*/

int lineNum(char *dictionaryName, char *word, int length) 
{
	char buffer[512] = {0};
	int dict_fd = open(dictionaryName, O_RDONLY);
	int search_high = lseek(dict_fd, 0, SEEK_END) / length;
	int search_low = 0;
	int file_index;
	int line_number;


	while (search_low <= search_high) {


		file_index = lseek(dict_fd, ((search_high + search_low) / 2) * length, SEEK_SET);
		line_number = file_index / length;
		read(dict_fd, buffer, length);

		// clean up the buffer by eliminating white space after the word
		int null_index = length - 1;

		do {
			if (buffer[--null_index] != ' ') {
				buffer[null_index + 1] = '\0';
				break;
			}
		} while (null_index);

		int comp = strcmp(buffer, word);

		if (comp == 0) {
			close(dict_fd);
			return line_number + 1;
		} else if (comp > 0) {
			search_high = line_number - 1;
		} else if (comp < 0) {
			search_low = line_number + 1;

		}
	} 

	close(dict_fd);
	return (line_number > 0) ? line_number * -1 : -1;
}