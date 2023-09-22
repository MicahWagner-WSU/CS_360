/***********************************************************************
name: Micah Wagner
	assignment4 -- acts as a pipe using ":" to seperate programs.
***********************************************************************/

/* Includes and definitions */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

/**********************************************************************
./assignment4 <arg1> : <arg2>

    Where: <arg1> and <arg2> are optional parameters that specify the programs
    to be run. If <arg1> is specified but <arg2> is not, then <arg1> should be
    run as though there was not a colon. Same for if <arg2> is specified but
    <arg1> is not.
**********************************************************************/


int main(int argc, char *argv[])
{
	int colon_index = 0;
	char **left;
	char **right;
	int fd[2];
	int rdr, wtr;

	// loop over each argv until we hit :, set colon index
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], ":") == 0) {
			colon_index = i;
			break;
		}
	}

	// the left arguement starts at the arguement after argv
	left = argv + 1;

	// the right arguement starts one after the colon index
	right = argv + colon_index + 1;

	// set colon index to null
	argv[colon_index] = NULL;

	// check user never gave colon, try to run left command
	if(colon_index == 0) {
		execvp(argv[1], left);
		fprintf(stderr, "%s\n", strerror(errno));
		exit(errno);
	}

	// user did give colon but not both args, run either left or right commands 
	if(right[0] == NULL && left[0] != NULL) {
		execvp(argv[1], left);
		fprintf(stderr, "%s\n", strerror(errno));
		exit(errno);
	} else if (right[0] != NULL && left[0] == NULL) {
		execvp(right[0], right);
		fprintf(stderr, "%s\n", strerror(errno));
		exit(errno);
	// user gave no args, only a colon
	} else if (right[0] == NULL && left[0] == NULL) {
		fprintf(stderr, "Usage: ./assignment4 <arg1> : <arg2> \n");
		exit(EXIT_FAILURE);
	}
	

	// create a pipe
	pipe(fd);

	//record reader and writer 
	rdr = fd[0]; wtr = fd[1];
	switch (fork()) {
		// fork failed
		case -1:
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		case 0:
			// we are child, close reader end since we're writing
			close(rdr);
			// close stdout and dup to connect filters
			close(1);
			dup(wtr); close(wtr);
			// exec and check if failed
			execvp(argv[1], left);
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		default:
			// we are parent, close writer since we're reading
			close(wtr);
			// close stdin and dup to connect filters
			close(0);
			dup(rdr); close(rdr);
			// exec and check if failed
			execvp(right[0], right);
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
	}
}

