#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>


/*
 * this function will recursively call itself and will 
 * return the amount of readable files in the given inputPath
 *
 * we assume that inputPath has already been checked for 
 * errors/special initial conditions like inputPath=null, 
 * and inputPath is some directory we cant read
 *
 * returns -errno if an error occured
 */
int recursive_check(char *inputPath);


/*
 * This function will check the initial conditions of inputPath
 * 
 * things we need to initially check for is if we are given null, 
 * or if the given file is a directory we cant read
 *
 * returns -errno if an error occured
 */
int readable(char *inputPath) 
{
	int copy_errno;
	DIR *test_dir;
	struct stat area, *s = &area;

	// check if we have bee given a null path
	if (inputPath == NULL) {
		char current_dir[PATH_MAX];

		//get the current directory and check for any errors
		if (getcwd(current_dir, PATH_MAX) == NULL) {
			copy_errno = errno;
			fprintf(stderr, "Error getting current working directory");
			return -copy_errno;
		} 	
		return recursive_check(current_dir);
	} 

	// get the status of the given file
	if (lstat(inputPath, s) == 0){
		// user inputed a directory
		if (S_ISDIR(s->st_mode)) {
			test_dir = opendir(inputPath);
			// we cannot open given file, return -errno
			if (test_dir == NULL) {
				copy_errno = errno;
				fprintf(stderr, "Error opening file");
				return -copy_errno;
			}
		}
	// lstat had an error, return -errno
	} else {
		copy_errno = errno;
		fprintf(stderr, "Error getting status of file");
		return -copy_errno;
	}

	closedir(test_dir);

	// input given was fine, count readable files
	return recursive_check(inputPath);
}


int recursive_check(char *inputPath)
{
	int copy_errno;
	int count = 0;
	DIR *working_dir;
	struct dirent *file_read;
	 
	struct stat area, *s = &area;

	// get the status of the given file
	if (lstat(inputPath, s) == 0){
		// base case 1 is that we hit a regular file
		if (S_ISREG(s->st_mode)) {
			if (s->st_mode & S_IRUSR)
				return 1;
			else
				//cant read, so dont increment count
				return 0;
		// base case 2 is that we hit a directory we cant read/cd into
		} else if (S_ISDIR(s->st_mode)    && 
			 (!(s->st_mode & S_IRUSR) || 
			  !(s->st_mode & S_IXUSR))) {
			return 0;
		// base case 3 is that we hit a special file
		} else if (!S_ISDIR(s->st_mode)){
			return 0;
		}
	// getting status of file failed for some reason 
	} else {
		copy_errno = errno;
		fprintf(stderr, "Error getting status of file");
		return -copy_errno;
	}

	// given path is a directory, return -errno if cant open
	working_dir = opendir(inputPath);
	if (working_dir == NULL) {
		copy_errno = errno;
		fprintf(stderr, "Error opening file");
		return -copy_errno;
	}

	// change directory into the path to use relative path names
	// return -errno if error occurs
	if (chdir(inputPath) == -1) {
		copy_errno = errno;
		fprintf(stderr, "Error changing directory");
		return -copy_errno;
	}

	int ret_val;
	// check if errno changes to see if readdir failed
	// we know readdir returns null if it fails or hits end of dir
	errno = 0;
	while ((file_read = readdir(working_dir)) != NULL) {
		// ignore the .. or . directory
		char *f_name = file_read->d_name;
		if (strcmp(f_name, "..")==0 || strcmp(f_name, ".")==0)
			continue;

		//count amount of readable regular files in given path
		ret_val = recursive_check(file_read->d_name);

		// check if error occured
		if (ret_val >= 0) {
			// increment number of readable regular files
			count += ret_val;
		} else {
			// error happened, return error code
			return ret_val;
		}
	}

	// errno changed, meaning readdir failed
	if (errno != 0) {
		copy_errno = errno;
		fprintf(stderr,"Error reading from file");
		return -copy_errno;
	}

	// done counting amount of regular files in this dir
	// change directory to parent and check for errors
	if (chdir("../") == -1) {
		copy_errno = errno;
		fprintf(stderr, "Error changing directory");
		return -copy_errno;
	}


	closedir(working_dir);
	// finished counting for this directory, return count
	return count;
}


