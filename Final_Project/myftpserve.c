#include "myftp.h"


int establish_serv_control_sock();

/*

things to do:
- create a function that handles each child process
- create generic function which spins up a new socket to accept (used fo data connection)
- also create generic function to close a socket



*/

int main(int argc, char **argv) {

	int listen_fd, connection_fd, length, child_pid, tmp_errno;
	struct sockaddr_in client_address;

	listen_fd = establish_serv_control_sock();

	if (listen_fd < 0)
		return -listen_fd;

	/* begin loop to coninually check for client connections */
	length = sizeof(struct sockaddr_in);
	for (;;) {
		/* occasionally attempt to clear zombies */
		while (waitpid(-1, NULL, WNOHANG) > 0);

		/* attempt to accept incoming connections */
		connection_fd = accept(listen_fd, (struct sockaddr *) &client_address, (socklen_t *) &length);

		if(connection_fd == -1) {
			tmp_errno = errno;
			perror("Error: ");
			exit(tmp_errno);
		}

		/* if we're the parent, keep checking for incoming connections and close connection */
		if (fork()) {
			close(connection_fd);
			continue;
		}

		/* the code from this line down is running on the child process */

		char client_name[NI_MAXHOST];
		int client_entry;

		/* attempt to retreive clients name */
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

		printf("Child %d: Connection accepted from host %s\n", getpid(), client_name);

		/* terminate connection with client */
		return 0;
	}
}

int establish_serv_control_sock() {
	int listen_fd, connection_fd, tmp_errno;
	struct sockaddr_in server_address;

	/* attempt to create tcp/ip socket */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_fd == -1) {
		tmp_errno = errno;
		perror("Error: ");
		return -tmp_errno;
	}

	/* this function clears the socket to make it available if the server is killed */
	tmp_errno = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error: ");
		return -tmp_errno;
	}

	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(MY_PORT_NUMBER);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	/* attempt to bind server address */
	tmp_errno = bind(listen_fd, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in));

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error: ");
		return -tmp_errno;
	}

	/* attempt to establish ourselves to listen for incomming conections */
	tmp_errno = listen(listen_fd, BACKLOG);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error: ");
		return -tmp_errno;
	}

	return listen_fd;
}
