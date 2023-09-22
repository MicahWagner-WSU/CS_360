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
	int colon_index;
	char **left;
	char **right;
	int fd[2];
	int rdr, wtr;

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], ":") == 0) {
			colon_index = i;
			break;
		}
	}
	left = argv + 1;
	right = argv + colon_index + 1;
	argv[colon_index] = NULL;

	pipe(fd);
	rdr = fd[0]; wtr = fd[1];
	if (fork()) {
		
		close(wtr);
		close(0);
		dup(rdr); close(rdr);
		execvp(right[0], right);

	} else {

		close(rdr);
		close(1);
		dup(wtr); close(wtr);
		execvp(argv[1], left);
	}

	return 0;
}

