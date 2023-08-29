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

	struct hash_table *table = hash_init(INITIAL_HASH_SIZE);

	char *key = malloc(sizeof(char) * 2);
	char *key1 = malloc(sizeof(char) * 2);
	char *key2 = malloc(sizeof(char) * 2);
	char *key3 = malloc(sizeof(char) * 2);
	char *key4 = malloc(sizeof(char) * 2);
	char *key5 = malloc(sizeof(char) * 2);
	char *key6 = malloc(sizeof(char) * 2);
	char *key7 = malloc(sizeof(char) * 2);
	char *key8 = malloc(sizeof(char) * 2);
	char *key9 = malloc(sizeof(char) * 2);
	char *key10 = malloc(sizeof(char) * 2);
	char *key11 = malloc(sizeof(char) * 2);

	strcpy(key, "A");
	strcpy(key1, "B");
	strcpy(key2, "C");
	strcpy(key3, "D");
	strcpy(key4, "E");
	strcpy(key5, "F");
	strcpy(key6, "G");
	strcpy(key7, "H");
	strcpy(key8, "I");
	strcpy(key9, "J");
	strcpy(key10, "K");
	strcpy(key11, "L");

	int *data = malloc(sizeof(int));
	*data = 0;

	int *data1 = malloc(sizeof(int));
	*data1 = 1;

	int *data2 = malloc(sizeof(int));
	*data2 = 2;

	int *data3 = malloc(sizeof(int));
	*data3 = 3;

	int *data4 = malloc(sizeof(int));
	*data4 = 4;

	int *data5 = malloc(sizeof(int));
	*data5 = 5;

	int *data6 = malloc(sizeof(int));
	*data6 = 6;

	int *data7 = malloc(sizeof(int));
	*data7 = 7;	

	int *data8 = malloc(sizeof(int));
	*data8 = 8;

	int *data9 = malloc(sizeof(int));
	*data9 = 9;

	int *data10 = malloc(sizeof(int));
	*data10 = 10;

	int *data11 = malloc(sizeof(int));
	*data11 = 11;

	hash_put_if_absent(table, key, data);
	hash_put_if_absent(table, key1, data1);
	hash_put_if_absent(table, key2, data2);
	hash_put_if_absent(table, key3, data3);
	hash_put_if_absent(table, key4, data4);
	hash_put_if_absent(table, key5, data5);
	hash_put_if_absent(table, key6, data6);
	hash_put_if_absent(table, key7, data7);
	hash_put_if_absent(table, key8, data8);
	hash_put_if_absent(table, key9, data9);
	hash_put_if_absent(table, key10, data10);
	hash_put_if_absent(table, key11, data11);
	printf("%d \n", *(int*)hash_put_if_absent(table, key, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key1, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key2, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key3, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key4, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key5, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key6, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key7, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key8, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key9, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key10, NULL));
	printf("%d \n", *(int*)hash_put_if_absent(table, key11, NULL));

	printf("%d", table->num_of_entries);

	hash_free(table);


	return 0;
}