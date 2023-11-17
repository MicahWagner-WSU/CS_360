#include "myftp.h"

int establish_client_connection(char *hostname);

char *get_input_line(int file_desc, int buf_size);


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
	char *args;

	socket_fd = establish_client_connection(hostname);

	if (socket_fd < 0)
		return -socket_fd;

	printf("Connected to server %s\n", hostname);

	printf("MFTP> ");
	fflush(stdout);

	for(;;) {

		// read input
		char *input = get_input_line(0, 1024);

		//should quit in here
		if (input == NULL) {
			free(input);
			return 0;
		}

		args = strtok(input, " ");
		while (args != NULL) {
			char ack[2] = {0};

			// stub for commands
			// anything in here is just for testing / getting used to things
			if(strcmp(args, "exit") == 0) {
				write(socket_fd, "Q\n", 2);
				read(socket_fd, &ack, 1);
				if(strcmp(ack, "A") == 0) {
					free(input);
					exit(0);
				}
			}
			if(strcmp(args, "cd") == 0)
				return 0;
			if(strcmp(args, "rcd") == 0)
				return 0;
			if(strcmp(args, "ls") == 0)
				return 0;
			if(strcmp(args, "rls") == 0)
				return 0;
			if(strcmp(args, "get") == 0)
				return 0;
			if(strcmp(args, "show") == 0)
				return 0;
			if(strcmp(args, "put") == 0)
				return 0;

			args = strtok(NULL, " ");

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
		fprintf(stderr, "Error: %s\n", gai_strerror(err));
		return -err;
	}

	/* attempt to create socket */
	socket_fd = socket(actual_data->ai_family, actual_data->ai_socktype, 0);

	if (socket_fd == -1) {
		tmp_errno = errno;
		perror("Error");
		return -tmp_errno;
	}

	/* attemp to connect to server */
	tmp_errno = connect(socket_fd, actual_data->ai_addr, actual_data->ai_addrlen);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error");
		return -tmp_errno;
	}

	return socket_fd;
}

char *get_input_line(int file_desc, int buf_size) {
	int actual = 0;
	int total_read = 0;
	char *final = calloc(MAX_ARG_LENGTH + 1, sizeof(char));
	char *buff = calloc(buf_size, sizeof(char));
	while((actual = read(file_desc, buff, buf_size)) > 0) {

		for (int i = 0; i < actual; i++) {
			if (buff[i] == '\n') {
				memcpy(&final[total_read], buff, buf_size);
				final[(total_read + actual) - 1] = '\0';
				return final;
			}
		}
		memcpy(&final[total_read], buff, buf_size);
		total_read += actual;
	}

	return NULL;
}

