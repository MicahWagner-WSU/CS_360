#include "myftp.h"


int establish_listening_sock(int port_num);

int handle_client(int control_connection_fd, struct sockaddr_in client_address);

char *get_input(int file_desc, int buf_size);

int handle_ctrl_cmd_D(int control_connection_fd);
int handle_ctrl_cmd_C(int control_connection_fd, char *path);
int handle_ctrl_cmd_L(int control_connection_fd, int data_fd);
int handle_ctrl_cmd_G(int control_connection_fd, int connected_data_fd, char *path);
int handle_ctrl_cmd_P(int control_connection_fd);
int handle_ctrl_cmd_Q(int control_connection_fd);

int send_ctrl_command(int socket_fd, char command, char *optional_message); 

/*


NOTE: probably get rid flushing, add a generic send_ctrl_command function, takes a command (A, E, L, etc.), next arguemnt is an optional string

*/

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
			perror("Error");
			exit(tmp_errno);
		}

		switch (fork()) {
			case -1:
				tmp_errno = errno;
				perror("Error");
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

int establish_listening_sock(int port_num) {
	int listen_fd, connection_fd, tmp_errno;
	struct sockaddr_in server_address;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_fd == -1) {
		tmp_errno = errno;
		perror("Error");
		return -tmp_errno;
	}

	tmp_errno = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error");
		return -tmp_errno;
	}

	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_num);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	tmp_errno = bind(listen_fd, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in));

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error");
		return -tmp_errno;
	}

	tmp_errno = listen(listen_fd, BACKLOG);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error");
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
		fprintf(stderr, "Error: %s\n", gai_strerror(client_entry));
		exit(client_entry);
	}

	printf("Child %d: Connection accepted from host %s\n", process_id, client_name);
	fflush(stdout);

	int connected_data_fd = 0;
	for(;;) {
		// for now ignore what the handler functions return, not sure if its necessary
		char *input = get_input(control_connection_fd, READ_BUF_LEN);
		if (input[0] == 'Q') {
			free(input);
			handle_ctrl_cmd_Q(control_connection_fd);
		} else if (input[0] == 'C') {
			handle_ctrl_cmd_C(control_connection_fd, &input[1]);
		} else if (input[0] == 'D') {
			connected_data_fd = handle_ctrl_cmd_D(control_connection_fd);
			if (connected_data_fd < 0)
				connected_data_fd = 0;
		} else if (input[0] == 'L') {
			if (connected_data_fd > 0) {
				handle_ctrl_cmd_L(control_connection_fd, connected_data_fd);
				connected_data_fd = 0;
			} else {
				send_ctrl_command(control_connection_fd, 'E', "Attempted 'ls' without establishing data connection");
			}
		} else if (input[0] == 'G') {
			if (connected_data_fd > 0) {
				handle_ctrl_cmd_G(control_connection_fd, connected_data_fd, &input[1]);
				connected_data_fd = 0;
			} else {
				send_ctrl_command(control_connection_fd, 'E', "Attempted 'get' without establishing data connection");
			}
		}

		free(input);

	}
	return 0;


}


int handle_ctrl_cmd_C(int control_connection_fd, char *path) {
	int tmp_errno;
	int cd_status;

	if (access(path, R_OK) != 0) {
		tmp_errno = errno;
		fprintf(stderr, "cd to %s failed with error message(a): %s\n", path, strerror(tmp_errno));
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return tmp_errno;
	}

	if ((cd_status = chdir(path)) == 0) {
		send_ctrl_command(control_connection_fd, 'A', NULL);

		printf("Child %d: changed directory to %s\n", getpid(), path);
		fflush(stdout);

		return 0;
	} else {
		tmp_errno = errno;
		fprintf(stderr, "cd to %s failed with error message(c): %s\n", path, strerror(tmp_errno));
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return tmp_errno;
	}
}


// error check first write, if so, do I error check write("E")?
int handle_ctrl_cmd_Q(int control_connection_fd) {
	int tmp_errno;
	if ((tmp_errno = send_ctrl_command(control_connection_fd, 'A', NULL)) != 0) {
		perror("Error writing");
		exit(tmp_errno);
	}

	printf("Child %d: Quitting\n", getpid());
	fflush(stdout);

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

	return connected_data_fd;

}

int handle_ctrl_cmd_L(int control_connection_fd, int data_fd) {
	int tmp_errno, status;

	send_ctrl_command(control_connection_fd, 'A', NULL);


	switch (fork()) {
		case -1:
			tmp_errno = errno;
			fprintf(stderr, "Error forking: %s\n", strerror(tmp_errno));
			exit(tmp_errno);
		case 0:
			// close stdin and dup to connect filters
			close(1);
			dup(data_fd);
			close(data_fd);

			// exec and check if failed
			execlp("ls", "ls", "-l", NULL);
			tmp_errno = errno;
			fprintf(stderr, "Error execing: %s\n", strerror(tmp_errno));
			exit(tmp_errno);
		default:
			break;
	}

	close(data_fd);

	if (wait(&status) == -1) {
		tmp_errno = errno;
		perror("Wait error");
		return tmp_errno;
	}	

	return 0;
}

int handle_ctrl_cmd_G(int control_connection_fd, int connected_data_fd, char *path) {
	int actual, tmp_errno, get_fd;
	struct stat area, *s = &area;
	char buff[READ_BUF_LEN];
	// get the status of the given file
	if (lstat(path, s) == 0){
		// base case 1 is that we hit a regular file
		if (S_ISREG(s->st_mode)) {
			// cehck if its readable by us
			if (access(path, R_OK) == -1) {
				send_ctrl_command(control_connection_fd, 'E', strerror(errno));	
				close(connected_data_fd);
				return -1;			
			} else {
				printf("Child %d: Reading file %s\n", getpid(), path);
			}

		} else if (S_ISDIR(s->st_mode)){
			send_ctrl_command(control_connection_fd, 'E', "File is a directory");
			close(connected_data_fd);
			return -1;
		} else {
			send_ctrl_command(control_connection_fd, 'E', "File is a special file");
			close(connected_data_fd);
			return -1;
		}
	// getting status of file failed for some reason 
	} else {
		tmp_errno = errno;
		perror("Error lstating");
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		close(connected_data_fd);
		return -1;
	}

	if ((get_fd = open(path, O_RDONLY)) == -1) {
		tmp_errno = errno;
		perror("Open/creating local file");
		send_ctrl_command(control_connection_fd, 'E', strerror(tmp_errno));
		return -1;
	}

	send_ctrl_command(control_connection_fd, 'A', NULL);

	while((actual = read(get_fd, buff, READ_BUF_LEN)) > 0) {
		write(connected_data_fd, buff, actual);
	}

	if (actual == -1) {
		perror("Open/creating local file");
		close(connected_data_fd);
		close(get_fd);
		return -1;
	}

	close(connected_data_fd);
	close(get_fd);

	return 0;
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

