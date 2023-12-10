#include "myftp.h"


int establish_listening_sock(int port_num);

int handle_client(int control_connection_fd, struct sockaddr_in client_address);

char *get_input(int file_desc, int buf_size);

int handle_ctrl_cmd_D(int control_connection_fd);
void handle_ctrl_cmd_C(int control_connection_fd, char *path);
void handle_ctrl_cmd_L(int control_connection_fd, int data_fd);
void handle_ctrl_cmd_G(int control_connection_fd, int connected_data_fd, char *path);
void handle_ctrl_cmd_P(int control_connection_fd, int connected_data_fd, char *path);
void handle_ctrl_cmd_Q(int control_connection_fd);

int send_ctrl_command(int socket_fd, char command, char *optional_message); 

int main(int argc, char **argv) {

	int listen_fd, connection_fd, length, child_pid, tmp_errno;
	struct sockaddr_in client_address;

	listen_fd = establish_listening_sock(MY_PORT_NUMBER);

	if (listen_fd < 0)
		return -listen_fd;

	length = sizeof(struct sockaddr_in);
	for (;;) {
		while (waitpid(-1, NULL, WNOHANG) > 0);

		connection_fd = accept(listen_fd, (struct sockaddr *) &client_address, (socklen_t *) &length);

		if(connection_fd == -1) {
			tmp_errno = errno;
			perror("Fatal accepting error");
			exit(tmp_errno);
		}

		switch (fork()) {
			case -1:
				tmp_errno = errno;
				perror("Fatal fork error");
				exit(tmp_errno);
			
			case 0:
				handle_client(connection_fd, client_address);
				break;

			default:
				close(connection_fd);
				continue;
		}

		return 0;
	}
}

// some of this code was taken from assignment 8 and slides
int establish_listening_sock(int port_num) {
	int listen_fd, connection_fd, tmp_errno;
	struct sockaddr_in server_address;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_fd == -1) {
		tmp_errno = errno;
		perror("Error creating socket");
		return -tmp_errno;
	}

	tmp_errno = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error setting socket options");
		return -tmp_errno;
	}

	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_num);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	tmp_errno = bind(listen_fd, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in));

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error binding socket");
		return -tmp_errno;
	}

	tmp_errno = listen(listen_fd, BACKLOG);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error setting socket to listen");
		return -tmp_errno;
	}

	return listen_fd;
}

int handle_client(int control_connection_fd, struct sockaddr_in client_address) {
	char client_name[NI_MAXHOST];
	int client_entry, actual, tmp_errno;
	int process_id = getpid();

	client_entry = getnameinfo(
		(struct sockaddr *) &client_address, 
		sizeof(struct sockaddr_in),
		client_name,
		sizeof(client_name),
		NULL,
		0,
		NI_NUMERICSERV);

	if (client_entry != 0) {
		fprintf(stderr, "Error getting client name: %s\n", gai_strerror(client_entry));
		exit(client_entry);
	}

	printf("Child %d: Connection accepted from host %s\n", process_id, client_name);

	int connected_data_fd = 0;
	for(;;) {

		char *input = get_input(control_connection_fd, READ_BUF_LEN);

		if (input == NULL) {
			tmp_errno = errno;
			if (tmp_errno == 0) {
				fprintf(stderr, "Child %d: Control Socket EOF detected, exiting \n", getpid());
				fprintf(stderr, "Child %d: Fatal error, exiting \n", getpid());
				exit(tmp_errno);
			} else {
				fprintf(stderr, "Child %d: Error reading from control socket, exiting \n", getpid());
				exit(tmp_errno);
			}
		}

		if (input[0] == 'Q') {
			free(input);
			handle_ctrl_cmd_Q(control_connection_fd);
		} else if (input[0] == 'C') {
			handle_ctrl_cmd_C(control_connection_fd, &input[1]);
		} else if (input[0] == 'D') {
			if (connected_data_fd > 0) {
				send_ctrl_command(control_connection_fd, 'E', "Unexpected D command, data connection exists");
				fprintf(stderr, "Child %d: Client issued redundant D command. Fatal error, exiting\n", getpid());
				exit(0);
			}

			connected_data_fd = handle_ctrl_cmd_D(control_connection_fd);
			if (connected_data_fd < 0)
				connected_data_fd = 0;
		} else if (input[0] == 'L') {
			if (connected_data_fd > 0) {
				handle_ctrl_cmd_L(control_connection_fd, connected_data_fd);
				connected_data_fd = 0;
			} else {
				send_ctrl_command(control_connection_fd, 'E', "Attempted 'rls' without establishing data connection");
			}
		} else if (input[0] == 'G') {
			if (connected_data_fd > 0) {
				handle_ctrl_cmd_G(control_connection_fd, connected_data_fd, &input[1]);
				connected_data_fd = 0;
			} else {
				send_ctrl_command(control_connection_fd, 'E', "Attempted 'get' without establishing data connection");
			}
		} else if (input[0] == 'P') {
			if (connected_data_fd > 0) {
				handle_ctrl_cmd_P(control_connection_fd, connected_data_fd, &input[1]);
				connected_data_fd = 0;
			} else {
				send_ctrl_command(control_connection_fd, 'E', "Attempted 'put' without establishing data connection");
			}
		}

		free(input);

	}
	return 0;


}


void handle_ctrl_cmd_C(int control_connection_fd, char *path) {
	int tmp_errno;
	int cd_status;

	if (access(path, R_OK) != 0) {
		tmp_errno = errno;
		fprintf(stderr, "cd to %s failed with error message(a): %s\n", path, strerror(tmp_errno));
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return;
	}

	if ((cd_status = chdir(path)) == 0) {
		send_ctrl_command(control_connection_fd, 'A', NULL);

		printf("Child %d: changed directory to %s\n", getpid(), path);

		return;
	} else {
		tmp_errno = errno;
		fprintf(stderr, "cd to %s failed with error message(c): %s\n", path, strerror(tmp_errno));
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return;
	}
}

void handle_ctrl_cmd_Q(int control_connection_fd) {
	send_ctrl_command(control_connection_fd, 'A', NULL);

	printf("Child %d: Quitting\n", getpid());

	exit(0);
}

int handle_ctrl_cmd_D(int control_connection_fd) {
	int tmp_errno, data_fd;
	struct sockaddr_in addr, client_address;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in ;
	char port_string[NI_MAXSERV + 2] = { 0 };

	data_fd = establish_listening_sock(0);

	if(data_fd < 0) {
		send_ctrl_command(control_connection_fd,'E',strerror(-data_fd));
		return data_fd;
	}

	memset(&addr, 0, sizeof(struct sockaddr));

	if (getsockname(data_fd, (struct sockaddr *) &addr, &addrlen) == -1) {
		tmp_errno = errno;
		perror("Error Retrieving Socket Name:");
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return -tmp_errno;
	}

	sprintf(port_string, "%d", ntohs(addr.sin_port));

	send_ctrl_command(control_connection_fd, 'A', port_string);

	int connected_data_fd = accept(data_fd, (struct sockaddr *) &client_address, (socklen_t *) &addrlen);

	if (connected_data_fd == -1) {
		tmp_errno = errno;
		perror("Error accepting data connection");
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return -tmp_errno;
	}

	close(data_fd);
	return connected_data_fd;

}

void handle_ctrl_cmd_L(int control_connection_fd, int data_fd) {
	int tmp_errno, status;

	send_ctrl_command(control_connection_fd, 'A', NULL);


	switch (fork()) {
		case -1:
			tmp_errno = errno;
			fprintf(stderr, "Error forking: %s\n", strerror(tmp_errno));
			exit(tmp_errno);
		case 0:

			close(1);
			dup(data_fd);
			close(data_fd);

			execlp("ls", "ls", "-l", NULL);
			tmp_errno = errno;
			fprintf(stderr, "Error execing: %s\n", strerror(tmp_errno));
			exit(tmp_errno);
		default:
			break;
	}

	close(data_fd);
	wait(NULL);
}

//some code was taken from my assingments
void handle_ctrl_cmd_G(int control_connection_fd, int connected_data_fd, char *path) {
	int actual, tmp_errno, get_fd;
	struct stat area, *s = &area;
	char buff[READ_BUF_LEN];

	if (lstat(path, s) == 0){

		if (S_ISREG(s->st_mode)) {

			if (access(path, R_OK) == -1) {
				send_ctrl_command(control_connection_fd, 'E', strerror(errno));	
				close(connected_data_fd);
				return;			
			} else {
				printf("Child %d: Reading file %s\n", getpid(), path);
			}

		} else if (S_ISDIR(s->st_mode)){
			send_ctrl_command(control_connection_fd, 'E', "File is a directory");
			close(connected_data_fd);
			return;
		} else {
			send_ctrl_command(control_connection_fd, 'E', "File is a special file");
			close(connected_data_fd);
			return;
		}

	} else {
		tmp_errno = errno;
		perror("Error lstating");
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		close(connected_data_fd);
		return;
	}

	if ((get_fd = open(path, O_RDONLY)) == -1) {
		tmp_errno = errno;
		perror("Open local file for reading");
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return;
	}

	send_ctrl_command(control_connection_fd, 'A', NULL);

	printf("Child %d: transmitting file %s to client\n", getpid(), path);

	signal(SIGPIPE, SIG_IGN);
	while((actual = read(get_fd, buff, READ_BUF_LEN)) > 0) {
		if (write(connected_data_fd, buff, actual) == -1) {
			perror("Error writing");
			close(connected_data_fd);
			close(get_fd);
			signal(SIGPIPE, SIG_DFL);
			return;
		}
	}

	if (actual == -1) {
		perror("Open/creating local file");
		close(connected_data_fd);
		close(get_fd);
		signal(SIGPIPE, SIG_DFL);
		return;
	}

	signal(SIGPIPE, SIG_DFL);
	close(connected_data_fd);
	close(get_fd);
}

void handle_ctrl_cmd_P(int control_connection_fd, int connected_data_fd, char *path) {
	int tmp_errno, new_fd, actual;
	char *tmp;
	char buff[READ_BUF_LEN];

	tmp = dirname(path);

	if (strcmp(tmp, ".") != 0) {
		send_ctrl_command(control_connection_fd, 'E', "Error, server was sent a pathname.  Base file name expected.");
		close(connected_data_fd);
		return;
	}

	if (access(path, F_OK) == 0){
		send_ctrl_command(control_connection_fd, 'E', "File exists");
		close(connected_data_fd);
		return;
	}

	if ((new_fd = open(path, O_CREAT|O_WRONLY|O_EXCL, 0644)) == -1) {
		tmp_errno = errno;
		perror("Open/creating local file");
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		close(connected_data_fd);
		return;
	}

	send_ctrl_command(control_connection_fd, 'A', NULL);

	printf("Child %d: receiving file %s from client\n", getpid(), path);

	while ((actual = read(connected_data_fd, buff, READ_BUF_LEN)) > 0) {
		write(new_fd, buff, actual);
	}

	if (actual == -1) {
		perror("Open/creating local file");
		close(connected_data_fd);
		close(new_fd);
		unlink(path);
		return;
	}

	close(connected_data_fd);
	close(new_fd);
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

