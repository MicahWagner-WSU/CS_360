#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "assignment3.h"


int readable(char *inputPath) 
{
	int copy_errno;
	int count = 0;
	DIR *working_dir;
	struct dirent *file_read;

	if(inputPath == NULL) {
		char current_dir[MAX_PATH_NAME];

		if(getcwd(current_dir, MAX_PATH_NAME) == NULL) {
			copy_errno = errno;
			perror("Error getting current working directory");
			return -copy_errno;
		} 	
		return readable(current_dir);
	} 


	 
	struct stat area, *s = &area;

	if(lstat(inputPath, s) == 0){
		// base case 1 is that we hit a regular file,
		if(S_ISREG(s->st_mode)) {
			if(s->st_mode & S_IRUSR)
				return 1;
			else
				return 0;
		// base case 2 is that we hit a directory we cannot read
		} else if(S_ISDIR(s->st_mode) && !(s->st_mode & S_IRUSR)) {
			return 0;
		// base case 3 is that we hit a special file
		} else if (!S_ISDIR(s->st_mode)){
			return 0;
		}
	} else {
		copy_errno = errno;
		fprintf(stderr, "Error getting status of file");
		return -copy_errno;
	}

	working_dir = opendir(inputPath);
	if(working_dir == NULL) {
		copy_errno = errno;
		fprintf(stderr, "Error opening file");
		return -copy_errno;
	}

	int ret_val;
	errno = 0;
	while((file_read = readdir(working_dir)) != NULL) {
		ret_val = readable(file_read->d_name);
		if(ret_val > 0) {
			count += ret_val;
		} else {
			return ret_val;
		}
	}

	if(errno != 0){
		copy_errno = errno;
		fprintf(stderr,"Error reading from file");
		return -copy_errno;
	}

	return count;


	// if((working_dir = opendir(inputPath)) != NULL) {
	// 	errno = 0;
	// 	file_read = readdir(working_dir);
	// 	if(errno != 0 && file_read == NULL) {
	// 		copy_errno = errno;
	// 		perror("Error reading directory");
	// 		return copy_errno;
	// 	} else if(file_read != NULL) {
	// 		if(check_if_dir(file_read->d_name)) {
	// 			readable(file_read->d_name);
	// 		}
	// 	}

	// } else {
	// // error check the open dir	
	// }
}


