#include "parse_utils.h"
#include "hash.h"

int main(int argc, char **argv) {

	int largest_count;
	int num_of_files;
	int file_argv_index;
	int largest_count_flag = sscanf(argv[1], "-%d", &largest_count);
	int i = 0;

	if(largest_count_flag > 0){

		num_of_files = argc - 2;
		file_argv_index = 2;

	} else if(largest_count_flag == 0){

		num_of_files = argc - 1;
		file_argv_index = 1;

	} else {

		exit(1); /* ask about this */

	}

	if(num_of_files < 0) exit(1);

	FILE *files[num_of_files];

	while(file_argv_index < num_of_files) {
		files[i] = fopen(argv[file_argv_index], "r");
		file_argv_index++;
		i++;
	}


	return 0;
}