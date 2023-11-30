#include "myftp.h"

int establish_socket_connection(char *hostname, int port_num);

char *get_input(int file_desc, int buf_size);

int manage_exit(int socket_fd);
int manage_cd(char *path);
int manage_rcd(int socket_fd, char *path);
int manage_ls();
int manage_rls(int socket_fd, char *hostname);
int manage_get(int socket_fd);
int manage_show(int socket_fd);
int manage_put(int socket_fd);

int establish_data_connection(int socket_fd, char *hostname);
int send_ctrl_command(int socket_fd, char command, char *optional_message);

/*

things to do:
fix: stop interchangeably doing string compares for 'A' and character compares, its dumb
fix: make a more robust acknowledgement function explicitly checking for A, E, or 0
fix: when cd takes no second arg, print "expecting a parameter"

NOTE: dont really need to error check stuff like write, close, dup, malloc, etc.

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

	socket_fd = establish_socket_connection(hostname, MY_PORT_NUMBER);

	if (socket_fd < 0)
		return -socket_fd;

	printf("Connected to server %s\n", hostname);

	printf("MFTP> ");
	fflush(stdout);

	int connected_data_fd = 0;
	for(;;) {

		// read input
		char *input = get_input(0, READ_BUF_LEN);


		//should quit in here, errno?
		if (input == NULL) {
			free(input);
			return 0;
		}

		first_arg = strtok(input, " \t\n");

		// stub for commands
		// anything in here is just for testing / getting used to things
		if (first_arg == NULL) {
			printf("MFTP> ");
			fflush(stdout);
			free(input);
			continue;
		} else if (strcmp(first_arg, "exit") == 0) {
			free(input);
			manage_exit(socket_fd);
		} else if (strcmp(first_arg, "cd") == 0) {
			second_arg = strtok(NULL, " ");
			//not sure what to do with error number
			//possibly print something else when we put no path
			manage_cd(second_arg);
		} else if (strcmp(first_arg, "rcd") == 0) {
			//not sure what to do with the return values here, nothing for now.
			second_arg = strtok(NULL, " ");
			manage_rcd(socket_fd,second_arg);

		} else if (strcmp(first_arg, "ls") == 0) {
			//maybe just exit if we get an error? all errors seem super extreme
			manage_ls();
		} else if (strcmp(first_arg, "rls") == 0) {
			// currently seg faults if we try to do this (data connection) when server quits unexpectedly, fix
			manage_rls(socket_fd, hostname);

		} else if (strcmp(first_arg, "get") == 0) {
			return 0;
		} else if (strcmp(first_arg, "show") == 0) {
			return 0;
		} else if (strcmp(first_arg, "put") == 0) {
			return 0;
		} else {
			printf("Command '%s' is unknown - ignored\n", first_arg);
		}
	
		printf("MFTP> ");
		fflush(stdout);
		free(input);
	}
}

int establish_socket_connection(char *hostname, int port_num) {
	int socket_fd, num_read, tmp_errno;
	struct addrinfo hints, *actual_data;
	char port_string[NI_MAXSERV] = { 0 };
	
	memset(&hints, 0, sizeof(hints));
	int err;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	sprintf(port_string, "%d", port_num);

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

	if ((tmp_errno = send_ctrl_command(socket_fd, 'Q', NULL)) != 0) {
		tmp_errno = errno;
		perror("Error sending exit");
		exit(errno); 
	}

	char *response = get_input(socket_fd, READ_BUF_LEN);

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
		printf("Error response from server: %s\n", &response[1]);
		free(response);
		exit(-1);
	}
}

int manage_cd(char *path) {

	int tmp_errno;
	int cd_status;

	if (access(path, R_OK) != 0) {
		tmp_errno = errno;
		perror("Change directory");
		return tmp_errno;
	}

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
					close(rdr);
					// close stdout and dup to connect filters
					close(1);
					dup(wtr);
					close(wtr);

					// exec and check if failed
					execlp("ls", "ls", "-l", NULL);
					fprintf(stderr, "%s\n", strerror(errno));
					exit(errno);
				} else {
					// we are parent, close writer since we're reading
					close(wtr);
					// close stdin and dup to connect filters
					close(0);
					dup(rdr);
					close(rdr);
					// exec and check if failed
					execlp("more", "more", "-20", NULL);
					fprintf(stderr, "%s\n", strerror(errno));
					exit(errno);
				}

			default:
				break;
		}
	}

	close(rdr);
	close(wtr);

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

int manage_rls(int socket_fd, char *hostname) {
	int tmp_errno, data_fd;

	data_fd = establish_data_connection(socket_fd, hostname);
	if (data_fd < 0) {
		close(data_fd);
		return -1;
	}

	send_ctrl_command(socket_fd, 'L', NULL);

	switch (fork()) {
		case -1:
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		case 0:
			// close stdin and dup to connect filters
			close(0);
			dup(data_fd);
			close(data_fd);

			// exec and check if failed
			execlp("more", "more", "-20", NULL);
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		default:
			break;
	}

	if (wait(NULL) == -1) {
		tmp_errno = errno;
		perror("Wait error");
		close(data_fd);
		return tmp_errno;
	}

	char *response = get_input(socket_fd, READ_BUF_LEN);

	if (response[0] == 'A') {
		free(response);
	} else {
		printf("Error response from server: %s\n", &response[1]);
		free(response);
		close(data_fd);
		return -1;
	}

	close(data_fd);
	return 0;

}

int manage_rcd(int socket_fd, char *path) {

	int tmp_errno;
	char *response;
// this feels silly, maybe dont do seperate write calls, kind of lazy
	send_ctrl_command(socket_fd, 'C', path);

	response = get_input(socket_fd, READ_BUF_LEN);

	if (strcmp(response, "A") == 0) {
		free(response);
		return 0;
	} else {
		printf("Error response from server: %s\n", &response[1]);
		free(response);
		return -1;
	}

}

int establish_data_connection(int socket_fd, char *hostname) {
	int tmp_errno;
	char *response;


	if (write(socket_fd, "D\n", 2) == -1) {
		tmp_errno = errno;
		perror("Error writing");
		return -errno; 
	}

	response = get_input(socket_fd, READ_BUF_LEN);

	if (response == NULL) {
		if(errno != 0) {
			tmp_errno = errno;
			perror("Input response error");
			free(response);
			exit(-errno); 
		} else {
			printf("Error: data socket closed unexpectedly\n");
			exit(-1);
		}
	}

	if (response[0] == 'A') {
		int port_num;

		sscanf(&response[1], "%d", &port_num);
		int data_fd = establish_socket_connection(hostname, port_num);

		if (socket_fd < 0)
			return socket_fd;

		return data_fd;

	} else {
		printf("Error response from server: %s\n", &response[1]);
		free(response);
		return -1;
	}


}


int send_ctrl_command(int socket_fd, char command, char *optional_message) {
	int tmp_errno;
	char *ctrl_command;
	if(optional_message != NULL) {
		ctrl_command = calloc(strlen(optional_message) + 3, sizeof(char));
		int message_size = strlen(optional_message);
		ctrl_command[0] = command;
		memcpy(&ctrl_command[1], optional_message, message_size);
		ctrl_command[message_size + 1] = '\n';
	} else {
		ctrl_command = calloc(3, sizeof(char));
		ctrl_command[0] = command;
		ctrl_command[1] = '\n';
	}

	if (write(socket_fd, ctrl_command, strlen(ctrl_command)) == -1) {
		tmp_errno = errno;
		perror("Error writing");
		return tmp_errno;
	}
	free(ctrl_command);
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



