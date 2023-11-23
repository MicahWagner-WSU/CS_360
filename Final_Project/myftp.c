#include "myftp.h"

int establish_client_connection(char *hostname);

char *get_input(int file_desc, int buf_size);

int manage_exit(int socket_fd);
int manage_cd(char *path);
int manage_rcd(int socket_fd);
int manage_ls();
int manage_rls(int socket_fd);
int manage_get(int socket_fd);
int manage_show(int socket_fd);
int manage_put(int socket_fd);

/*

things to do:


- make a clean error handle function (later)
- make it so that I can read input buffer of length 2 rn it halts
- handle the EOF case 
- make sure you terminate commands with new line
- create a seperate function for parsing input
	- either create a function for each individual command *
	- or create a function that raps the whole token parsing 
- create generic function that spins up new socket (used for data connection)
- also create generic function to close a socket


*/

int main(int argc, char **argv) {
	int socket_fd, actual, tmp_errno;
	char *hostname = argv[1];
	char buff[MAX_ARG_LENGTH + 1] = { 0 };
	char *first_arg;
	char *second_arg;

	if (argc != 2) {
		printf("Usage: ./mftp <hostname | IP address>\n");
		return -1;
	}

	socket_fd = establish_client_connection(hostname);

	if (socket_fd < 0)
		return -socket_fd;

	printf("Connected to server %s\n", hostname);

	printf("MFTP> ");
	fflush(stdout);

	for(;;) {

		// read input
		char *input = get_input(0, READ_BUF_LEN);

		//should quit in here, errno?
		if (input == NULL) {
			free(input);
			return 0;
		}

		first_arg = strtok(input, " ");


		// stub for commands
		// anything in here is just for testing / getting used to things
		if(strcmp(first_arg, "exit") == 0) {
			free(input);
			manage_exit(socket_fd);
		} else if(strcmp(first_arg, "cd") == 0) {
			second_arg = strtok(NULL, " ");
			//not sure what to do with error number
			//possibly print something else when we put no path
			manage_cd(second_arg);
		} else if(strcmp(first_arg, "rcd") == 0) {
			return 0;
		} else if(strcmp(first_arg, "ls") == 0) {
			//maybe just exit if we get an error? all errors seem super extreme
			manage_ls();
		} else if(strcmp(first_arg, "rls") == 0) {
			return 0;
		} else if(strcmp(first_arg, "get") == 0) {
			return 0;
		} else if(strcmp(first_arg, "show") == 0) {
			return 0;
		} else if(strcmp(first_arg, "put") == 0) {
			return 0;
		} else {
			printf("Command '%s' is unknown - ignored\n", first_arg);
		}
	
		printf("MFTP> ");
		fflush(stdout);
		free(input);
	}
}

int establish_client_connection(char *hostname) {
	int socket_fd, num_read, tmp_errno;
	struct addrinfo hints, *actual_data;
	char port_string[NI_MAXSERV] = { 0 };
	
	memset(&hints, 0, sizeof(hints));
	int err;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	sprintf(port_string, "%d", MY_PORT_NUMBER);

	/* first get the server address info */
	err = getaddrinfo(hostname, port_string, &hints, &actual_data);

	if (err != 0) {
		fprintf(stderr, "Connection to server failed: %s\n", gai_strerror(err));
		return -err;
	}

	/* attempt to create socket */
	socket_fd = socket(actual_data->ai_family, actual_data->ai_socktype, 0);

	if (socket_fd == -1) {
		tmp_errno = errno;
		perror("Connection to server failed");
		return -tmp_errno;
	}

	/* attemp to connect to server */
	tmp_errno = connect(socket_fd, actual_data->ai_addr, actual_data->ai_addrlen);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Connection to server failed");
		return -tmp_errno;
	}

	return socket_fd;
}
// 0 success, above 0 errno from client, below 0 error from server
int manage_exit(int socket_fd) {
	int tmp_errno;

	if (write(socket_fd, "Q\n", 2) == -1) {
		tmp_errno = errno;
		perror("Error writing");
		exit(errno); 
	}

	char *response = get_input(socket_fd, 2);

	if (response == NULL) {
		if (errno) {
			tmp_errno = errno;
			perror("Error reading");
			exit(errno); 
		} else {
			printf("Error: control socket closed unexpectedly\n");
			exit(-1);
		}

	}

	if (strcmp(response, "A") == 0) {
		free(response);
		exit(0);
	} else {
		printf("%s\n", &response[1]);
		free(response);
		exit(-1);
	}
}

int manage_cd(char *path) {

	int tmp_errno;
	int cd_status;

	if ((cd_status = chdir(path)) == 0) {
		return 0;
	} else {
		tmp_errno = errno;
		perror("Change directory");
		return tmp_errno;
	}
}

//verbose error checking, ask if I need it
int manage_ls() {

	int fd[2];
	int rdr, wtr, tmp_errno;

	if (pipe(fd) == -1) {
		tmp_errno = errno;
		perror("Pipe error");
		return tmp_errno;
	}

	rdr = fd[0]; wtr = fd[1];

	for(int i = 0; i < 2; i++) {

		switch (fork()) {
			case -1:
				fprintf(stderr, "%s\n", strerror(errno));
				exit(errno);
			case 0:
				if (i == 0) {
					// we are child, close reader end since we're writing
					if (close(rdr) == -1) {
						tmp_errno = errno;
						perror("Close error");
						exit(tmp_errno);
					}
					// close stdout and dup to connect filters
					if (close(1) == -1) {
						tmp_errno = errno;
						perror("Close error");
						exit(tmp_errno);
					}

					if (dup(wtr) == -1) {
						tmp_errno = errno;
						perror("FD duplication error");
						exit(tmp_errno);
					}
					
					if (close(wtr) == -1) {
						tmp_errno = errno;
						perror("Close error");
						exit(tmp_errno);
					}
					// exec and check if failed
					execlp("ls", "ls", "-l", NULL);
					fprintf(stderr, "%s\n", strerror(errno));
					exit(errno);
				} else {
					// we are parent, close writer since we're reading
					if (close(wtr) == -1) {
						tmp_errno = errno;
						perror("Close error");
						exit(tmp_errno);
					}
					// close stdin and dup to connect filters
					if (close(0) == -1) {
						tmp_errno = errno;
						perror("Close error");
						exit(tmp_errno);
					}

					if (dup(rdr) == -1) {
						tmp_errno = errno;
						perror("FD duplication error");
						exit(tmp_errno);
					} 

					if (close(rdr) == -1) {
						tmp_errno = errno;
						perror("Close error");
						exit(tmp_errno);
					}
					// exec and check if failed
					execlp("more", "more", "-20", NULL);
					fprintf(stderr, "%s\n", strerror(errno));
					exit(errno);
				}

			default:
				break;
		}
	}

	if (close(rdr) == -1) {
		tmp_errno = errno;
		perror("Close error");
		return tmp_errno;
	}

	if (close(wtr) == -1) {
		tmp_errno = errno;
		perror("Close error");
		return tmp_errno;
	}

	if (wait(NULL) == -1) {
		tmp_errno = errno;
		perror("Wait error");
		return tmp_errno;
	}

	if (wait(NULL) == -1) {
		tmp_errno = errno;
		perror("Wait error");
		return tmp_errno;
	}

	return 0;
}

char *get_input(int file_desc, int buf_size) {
	int actual = 0;
	int total_read = 0;
	char *final = calloc(MAX_ARG_LENGTH + 1, sizeof(char));
	char *buff = calloc(buf_size, sizeof(char));
	while((actual = read(file_desc, buff, buf_size)) > 0) {
		for (int i = 0; i < actual; i++) {
			if (buff[i] == '\n' || i > MAX_ARG_LENGTH + 1) {
				memcpy(&final[total_read], buff, actual);
				final[(total_read + actual) - 1] = '\0';
				free(buff);
				return final;
			}
		}
		memcpy(&final[total_read], buff, buf_size);
		total_read += actual;
	}

	free(buff);
	free(final);
	return NULL;

}



