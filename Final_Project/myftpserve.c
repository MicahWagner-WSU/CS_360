#include "myftp.h"


int establish_listening_sock(int port_num);

int handle_client(int control_connection_fd, struct sockaddr_in client_address);

char *get_input(int file_desc, int buf_size);

int handle_ctrl_cmd_D(int control_connection_fd);
int handle_ctrl_cmd_C(int control_connection_fd, char *path);
int handle_ctrl_cmd_L(int control_connection_fd);
int handle_ctrl_cmd_G(int control_connection_fd);
int handle_ctrl_cmd_P(int control_connection_fd);
int handle_ctrl_cmd_Q(int control_connection_fd);

int send_error(int socket_fd, int error_num);

/*

things to do:
- create a read loop to read control commands from client
- create generic function which spins up a new socket to accept (used fo data connection)
- also create generic function to close a socket

NOTE: probably get rid flushing

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

	for(;;) {
		// for now ignore what the handler functions return, not sure if its necessary
		char *input = get_input(control_connection_fd, READ_BUF_LEN);
		if (strcmp(input, "Q") == 0) {
			free(input);
			handle_ctrl_cmd_Q(control_connection_fd);
		} else if (input[0] == 'C') {
			handle_ctrl_cmd_C(control_connection_fd, &input[1]);
		} else if (input[0] == 'D') {

		}

		free(input);

	}
	return 0;


}


int handle_ctrl_cmd_C(int control_connection_fd, char *path) {
	int tmp_errno;
	int cd_status;

	if ((cd_status = chdir(path)) == 0) {
		if (write(control_connection_fd, "A\n", 2) == -1) {
			tmp_errno = errno;
			perror("Error writing");
			return tmp_errno;
		}

		printf("Child %d: changed directory to %s\n", getpid(), path);
		fflush(stdout);

		return 0;
	} else {
		tmp_errno = errno;
		perror("Change directory error");
		send_error(control_connection_fd, tmp_errno);
		return tmp_errno;
	}
}


// error check first write, if so, do I error check write("E")?
int handle_ctrl_cmd_Q(int control_connection_fd) {
	int tmp_errno;
	if (write(control_connection_fd, "A\n", 2) == -1) {
		tmp_errno = errno;
		perror("Error writing");
		exit(tmp_errno);
	}

	printf("Child %d: Quitting\n", getpid());
	fflush(stdout);

	exit(0);
}

int handle_ctrl_cmd_D(int control_connection_fd) {
	int tmp_errno, data_fd;
	struct sockaddr_in addr = {0};
	socklen_t addrlen = sizeof(struct sockaddr_in);
	char port_string[NI_MAXSERV] = { 0 };


	data_fd = establish_listening_sock(0);

	if(data_fd < 0) {
		send_error(control_connection_fd, -data_fd);
		return data_fd;
	}

	if (write(control_connection_fd, "A", 1) == -1) {
		tmp_errno = errno;
		perror("Error writing");
		return -tmp_errno;
	}

	if (getsockname(data_fd, (struct sockaddr *) &addr, &addrlen) == -1) {
		tmp_errno = errno;
		perror("Error Retrieving Socket Name:");
		send_error(control_connection_fd, tmp_errno);
		return -tmp_errno;
	}

	sprintf(port_string, "%d", ntohs(addr.sin_port));

	//send port? how should do?
	// if (write(socket_fd, port_string, ?) == -1) {
	// 	tmp_errno = errno;
	// 	perror("Error writing");
	// 	return -tmp_errno;
	// }

}

int send_error(int socket_fd, int error_num) {
	int tmp_errno;
	char *string_error = strerror(error_num);
// this feels silly, try not to do seperate write calls, feels lazy
	if (write(socket_fd, "E", 1) == -1) {
		tmp_errno = errno;
		perror("Error writing");
		return tmp_errno;
	}

	if (write(socket_fd, string_error, strlen(string_error)) == -1) {
		tmp_errno = errno;
		perror("Error writing");
		return tmp_errno;
	}
	
	if (write(socket_fd, "\n", 1) == -1) {
		tmp_errno = errno;
		perror("Error writing");
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

	if(actual == -1) {
		free(buff);
		free(final);
		return NULL;
	}
}

