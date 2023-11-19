#include "myftp.h"


int establish_serv_control_sock();

int handle_client(int control_connection_fd, struct sockaddr_in client_address);

char *get_input(int file_desc, int buf_size);

int handle_ctrl_cmd_D(int control_connection_fd);
int handle_ctrl_cmd_C(int control_connection_fd);
int handle_ctrl_cmd_L(int control_connection_fd);
int handle_ctrl_cmd_G(int control_connection_fd);
int handle_ctrl_cmd_P(int control_connection_fd);
int handle_ctrl_cmd_Q(int control_connection_fd);

/*

things to do:
- create a read loop to read control commands from client
- create generic function which spins up a new socket to accept (used fo data connection)
- also create generic function to close a socket



*/

int main(int argc, char **argv) {

	int listen_fd, connection_fd, length, child_pid, tmp_errno;
	struct sockaddr_in client_address;

	listen_fd = establish_serv_control_sock();

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

int establish_serv_control_sock() {
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
	server_address.sin_port = htons(MY_PORT_NUMBER);
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
		char *input = get_input(control_connection_fd, READ_BUF_LEN);
		if(strcmp(input, "Q") == 0) {
			free(input);
			if (handle_ctrl_cmd_Q(control_connection_fd) == 0) {
				return 0;
			} else {
				tmp_errno = errno;
				perror("Error");
				return tmp_errno;
			}
		}

	}
	return 0;


}

int handle_ctrl_cmd_Q(int control_connection_fd) {
	if (write(control_connection_fd, "A\n", 2) == -1) 
		return errno;

	printf("Child %d: Quitting\n", getpid());
	fflush(stdout);

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

