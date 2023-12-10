#include "myftp.h"

int establish_socket_connection(char *hostname, int port_num);

char *get_input(int file_desc, int buf_size);

void manage_exit(int socket_fd);
void manage_cd(char *path);
void manage_rcd(int socket_fd, char *path);
void manage_ls();
void manage_rls(int socket_fd, char *hostname);
void manage_get(int socket_fd, char *hostname, char *path);
void manage_show(int socket_fd, char *hostname, char *path);
void manage_put(int socket_fd, char *hostname, char *path);

int establish_data_connection(int socket_fd, char *hostname);
int send_ctrl_command(int socket_fd, char command, char *optional_message);
int manage_response(int socket_fd);

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

		char *input = get_input(0, READ_BUF_LEN);

		if (input == NULL) {
			free(input);
			return 0;
		}

		first_arg = strtok(input, " \t\n");

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

			if (second_arg == NULL) {
				fprintf(stderr, "Command error: expecting a parameter.\n");
				printf("MFTP> ");
				fflush(stdout);
				free(input);
				continue;
			}
			manage_cd(second_arg);
		} else if (strcmp(first_arg, "rcd") == 0) {

			second_arg = strtok(NULL, " ");
			if (second_arg == NULL) {
				fprintf(stderr, "Command error: expecting a parameter.\n");
				printf("MFTP> ");
				fflush(stdout);
				free(input);
				continue;
			}
			manage_rcd(socket_fd,second_arg);

		} else if (strcmp(first_arg, "ls") == 0) {

			manage_ls();

		} else if (strcmp(first_arg, "rls") == 0) {

			manage_rls(socket_fd, hostname);

		} else if (strcmp(first_arg, "get") == 0) {
			second_arg = strtok(NULL, " ");
			if (second_arg == NULL) {
				fprintf(stderr, "Command error: expecting a parameter.\n");
				printf("MFTP> ");
				fflush(stdout);
				free(input);
				continue;
			}
			manage_get(socket_fd, hostname, second_arg);
		} else if (strcmp(first_arg, "show") == 0) {
			second_arg = strtok(NULL, " ");
			if (second_arg == NULL) {
				fprintf(stderr, "Command error: expecting a parameter.\n");
				printf("MFTP> ");
				fflush(stdout);
				free(input);
				continue;
			}
			manage_show(socket_fd, hostname, second_arg);
		} else if (strcmp(first_arg, "put") == 0) {
			second_arg = strtok(NULL, " ");
			if (second_arg == NULL) {
				fprintf(stderr, "Command error: expecting a parameter.\n");
				printf("MFTP> ");
				fflush(stdout);
				free(input);
				continue;
			}
			manage_put(socket_fd, hostname, second_arg);
		} else {
			printf("Command '%s' is unknown - ignored\n", first_arg);
		}
	
		printf("MFTP> ");
		fflush(stdout);
		free(input);
	}
}

// some of this code was taken from assignment 8 and slides
int establish_socket_connection(char *hostname, int port_num) {
	int socket_fd, num_read, tmp_errno;
	struct addrinfo hints, *actual_data;
	char port_string[NI_MAXSERV] = { 0 };
	
	memset(&hints, 0, sizeof(hints));
	int err;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	sprintf(port_string, "%d", port_num);

	err = getaddrinfo(hostname, port_string, &hints, &actual_data);

	if (err != 0) {
		fprintf(stderr, "Connection to server failed: %s\n", gai_strerror(err));
		return -err;
	}

	socket_fd = socket(actual_data->ai_family, actual_data->ai_socktype, 0);

	if (socket_fd == -1) {
		tmp_errno = errno;
		perror("Connection to server failed");
		return -tmp_errno;
	}

	tmp_errno = connect(socket_fd, actual_data->ai_addr, actual_data->ai_addrlen);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Connection to server failed");
		return -tmp_errno;
	}

	return socket_fd;
}

void manage_exit(int socket_fd) {
	send_ctrl_command(socket_fd, 'Q', NULL);
	manage_response(socket_fd);
	exit(0);
}

void manage_cd(char *path) {

	int tmp_errno;
	int cd_status;

	if (access(path, R_OK) != 0) {
		tmp_errno = errno;
		perror("Change directory");
		return;
	}

	if ((cd_status = chdir(path)) == 0) {
		return;
	} else {
		tmp_errno = errno;
		perror("Change directory");
		return;
	}
}


void manage_ls() {

	int fd[2];
	int rdr, wtr;

	if (pipe(fd) == -1) {
		perror("Pipe error");
		exit(errno);
	}

	rdr = fd[0]; wtr = fd[1];

	for(int i = 0; i < 2; i++) {

		switch (fork()) {
			case -1:
				fprintf(stderr, "%s\n", strerror(errno));
				exit(errno);
			case 0:
				if (i == 0) {

					close(rdr);

					close(1);
					dup(wtr);
					close(wtr);

					execlp("ls", "ls", "-l", NULL);
					fprintf(stderr, "%s\n", strerror(errno));
					exit(errno);
				} else {
					close(wtr);

					close(0);
					dup(rdr);
					close(rdr);

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
	wait(NULL);
	wait(NULL);
}

void manage_rls(int socket_fd, char *hostname) {
	int data_fd;

	data_fd = establish_data_connection(socket_fd, hostname);
	if (data_fd < 0) {
		close(data_fd);
		return;
	}

	send_ctrl_command(socket_fd, 'L', NULL);

	if (manage_response(socket_fd) != 0) {
		close(data_fd);
		return;
	}
	

	switch (fork()) {
		case -1:
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		case 0:

			close(0);
			dup(data_fd);
			close(data_fd);

			execlp("more", "more", "-20", NULL);
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		default:
			break;
	}

	close(data_fd);

	wait(NULL);
}

void manage_rcd(int socket_fd, char *path) {
	char *response;

	send_ctrl_command(socket_fd, 'C', path);

	manage_response(socket_fd);

}

void manage_get(int socket_fd, char* hostname, char *path) {
	int data_fd, new_fd, actual;
	char buff[READ_BUF_LEN];
	char *response;
	char *base_path = basename(path);

	if ((new_fd = open(base_path, O_CREAT|O_WRONLY|O_EXCL, 0644)) == -1) {
		perror("Open/creating local file");
		return;
	}

	data_fd = establish_data_connection(socket_fd, hostname);
	if (data_fd < 0) {
		close(data_fd);
		close(new_fd);
		unlink(base_path);
		return;
	}

	send_ctrl_command(socket_fd, 'G', path);

	if (manage_response(socket_fd) != 0) {
		close(data_fd);
		close(new_fd);
		unlink(base_path);
		return;
	}

	while ((actual = read(data_fd, buff, READ_BUF_LEN)) > 0) {
		write(new_fd, buff, actual);
	}

	if (actual == -1) {
		perror("Open/creating local file");
		close(data_fd);
		close(new_fd);
		unlink(base_path);
		return;
	}

	close(data_fd);
	close(new_fd);
}

void manage_show(int socket_fd, char *hostname, char *path) {
	char *response;
	int data_fd;

	data_fd = establish_data_connection(socket_fd, hostname);
	if (data_fd < 0) {
		close(data_fd);
		return;
	}

	send_ctrl_command(socket_fd, 'G', path);

	if (manage_response(socket_fd) != 0) {
		close(data_fd);
		return;
	}

	switch (fork()) {
		case -1:
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		case 0:

			close(0);
			dup(data_fd);
			close(data_fd);

			execlp("more", "more", "-20", NULL);
			fprintf(stderr, "%s\n", strerror(errno));
			exit(errno);
		default:
			break;
	}

	close(data_fd);
	wait(NULL);
}

void manage_put(int socket_fd, char* hostname, char *path) {
	int data_fd, put_fd, actual;
	char buff[READ_BUF_LEN];
	struct stat area, *s = &area;
	char *response;
	char *base_path = basename(path);

	if (access(path, F_OK) != 0){
		perror("Opening file for reading");
		return;
	}

	if (lstat(path, s) == 0){

		if (!S_ISREG(s->st_mode) || S_ISDIR(s->st_mode)) {
			fprintf(stderr, "Local file comp is a directory, command ignored.\n");
			return;
		}

	} else {
		perror("Error lstating");
		return;
	}

	if ((put_fd = open(path, O_RDONLY)) == -1) {
		perror("Opening file for reading");
		return;
	}

	data_fd = establish_data_connection(socket_fd, hostname);
	if (data_fd < 0) {
		close(put_fd);
		close(data_fd);
		return;
	}

	send_ctrl_command(socket_fd, 'P', base_path);

	if (manage_response(socket_fd) != 0) {
		close(put_fd);
		close(data_fd);
		return;
	}

	while ((actual = read(put_fd, buff, READ_BUF_LEN)) > 0) {
		write(data_fd, buff, actual);
	}

	if (actual == -1) {
		perror("reading from file");
		close(data_fd);
		close(put_fd);
		return;
	}

	close(data_fd);
	close(put_fd);
}

int establish_data_connection(int socket_fd, char *hostname) {
	int tmp_errno, data_fd, port_num;
	char *response;

	send_ctrl_command(socket_fd, 'D', NULL);

	if ((port_num = manage_response(socket_fd)) < 0) {
		return -1;
	}

	data_fd = establish_socket_connection(hostname, port_num);

	if (data_fd < 0)
		return data_fd;

	return data_fd;


}

int manage_response(int socket_fd) {

	char *response = get_input(socket_fd, READ_BUF_LEN);
	int port_num;


	if (response == NULL) {
		if (errno) {
			perror("Error reading");
			exit(EXIT_FAILURE); 
		} else {
			printf("Error: control socket closed unexpectedly\n");
			exit(EXIT_FAILURE);
		}

	}

	if (response[0] == 'A') {
		if(strlen(response) != 1) {
			sscanf(&response[1], "%d", &port_num);
			free(response);
			return port_num;
		}

		free(response);
		return 0;
	} else if (response[0] == 'E') {
		fprintf(stderr, "Error response from server: %s\n", &response[1]);
		free(response);
		return -1;
	} else {
		fprintf(stderr, "Expected E or A from server, instead received %s\n", response);
		free(response);
		exit(EXIT_FAILURE);
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
		free(ctrl_command);
		exit(tmp_errno);
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



