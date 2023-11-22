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

			int exit_status;
			if((exit_status = manage_exit(socket_fd)) == 0) {
				exit(0);
			} else {
				exit(exit_status);
			}
		} else if(strcmp(first_arg, "cd") == 0) {
			second_arg = strtok(NULL, " ");
			manage_cd(second_arg);
		} else if(strcmp(first_arg, "rcd") == 0) {
			return 0;
		} else if(strcmp(first_arg, "ls") == 0) {
			return 0;
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

		//first_arg = strtok(NULL, " ");

	
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
		return errno; 
	}

	char *response = get_input(socket_fd, 2);

	if (response == NULL) {
		if (errno) {
			tmp_errno = errno;
			perror("Error reading");
			return errno; 
		} else {
			printf("Error: control socket closed unexpectedly\n");
			return -1;
		}

	}

	if (strcmp(response, "A") == 0) {
		free(response);
		return 0;
	} else {
		printf("%s\n", &response[1]);
		free(response);
		return -1;
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



