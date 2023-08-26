// maybe add comment block and main.h include?
#include <stdlib.h>
#include "parse_utils.h"
#include "hash.h"

int main(int argc, char **argv) {

	// int largest_count;
	// int num_of_files;
	// int file_argv_index;
	// int largest_count_flag;
	// int i = 0;

	// if(argc <= 1) exit(1);

	// largest_count_flag = sscanf(argv[1], "-%d", &largest_count);

	// if(largest_count_flag > 0){

	// 	printf("count flag is here %d", largest_count);
	// 	num_of_files = argc - 2;
	// 	file_argv_index = 2;

	// } else if(largest_count_flag == 0){

	// 	printf("count flag not here");
	// 	num_of_files = argc - 1;
	// 	file_argv_index = 1;

	// } else {

	// 	exit(1); /* ask about this */

	// }

	// if(num_of_files <= 0) exit(1);

	// FILE *files[num_of_files];

	// while(file_argv_index < num_of_files) {
	// 	files[i] = fopen(argv[file_argv_index], "r");
	// 	file_argv_index++;
	// 	i++;
	// }

	//struct hash_entry *table = hash_init();

	struct key_value *list = list_init();

	int *data = malloc(sizeof(int));
	*data = 5;

	char *key = malloc(sizeof(char) * 2);
	key[0] = 'X';
	key[1] = '\0';

	int *data1 = malloc(sizeof(int));
	*data1 = 24795;

	char *key1 = malloc(sizeof(char) * 2);
	key1[0] = 'Y';
	key1[1] = '\0';


	list_add(list, key, (void *)data);
	list_add(list, key1, (void *)data1);
	printf("list keys: %s, %s, %s", list->key, list->next->key, list->next->next->key);
	printf("%d \n", * (int *)list_get(list, key1));

	list_free(list);


	return 0;
}