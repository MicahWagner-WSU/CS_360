#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define MY_PORT_NUMBER 49999
#define DATE_SIZE 18
#define BACKLOG 1


/*
 * this function creates a server and listens for incoming connections
 *
 * It first creates a AF_INET socket using the tcp protocol,
 * then binds and listens on this socket. When someone connects,
 * we send back the date and time, and print the connected user 
 * and total amount of connected users.
 *
 * when we receive a connection, we create a child process to handle that connection
 * that child process quites after sending the date and time
 */
void run_server();

/*
 * this function attempts to connect to hostname using the sockets api
 *
 * It first creates a AF_INET socket using the tcp protocol,
 * then attempts to connect to the given host name
 *
 * after a successful connection, the client reads and prints a date from the server
 */
void run_client(const char *hostname);

int main(int argc, char const *argv[]){

	if (argc > 1) {

		/* user specified client, run client code */
		if (strcmp(argv[1], "client") == 0) {
			run_client(argv[2]);

		/* user specified server, run server code */
		} else if (strcmp(argv[1], "server") == 0){
			run_server();

		/* user specified incorrect arguemnts */
		} else {
			printf("USAGE: <client/server> <hostname>");
		}
	} else {
		printf("USAGE: <client/server> <hostname>");
	}
    return 0;
}

void run_client(const char *hostname) {
	int socket_fd, num_read, tmp_errno;
	struct addrinfo hints, *actual_data;
	char port_string[NI_MAXSERV] = { 0 };
	char buff[DATE_SIZE + 1] = { 0 };
	
	memset(&hints, 0, sizeof(hints));
	int err;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	sprintf(port_string, "%d", MY_PORT_NUMBER);

	/* first get the server address info */
	err = getaddrinfo(hostname, port_string, &hints, &actual_data);

	if (err != 0) {
		fprintf(stderr, "Error: %s\n", gai_strerror(err));
		exit(err);
	}

	/* attempt to create socket */
	socket_fd = socket(actual_data->ai_family, actual_data->ai_socktype, 0);

	if (socket_fd == -1) {
		tmp_errno = errno;
		perror("Error: ");
		exit(tmp_errno);
	}

	/* attemp to connect to server */
	tmp_errno = connect(socket_fd, actual_data->ai_addr, actual_data->ai_addrlen);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error: ");
		exit(tmp_errno);
	}

	/* attempt to read from the server */
	if ((num_read = read(socket_fd, buff, DATE_SIZE)) == -1) {
		tmp_errno = errno;
		perror("Error: ");
		exit(tmp_errno);
	}

	/* print the date we received */
	printf("%s\n", buff);
}

void run_server() {
	int listen_fd, connection_fd, length, clients_connected, tmp_errno;
	struct sockaddr_in server_address, client_address;

	/* attempt to create tcp/ip socket */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_fd == -1) {
		tmp_errno = errno;
		perror("Error: ");
		exit(tmp_errno);
	}

	/* this function clears the socket to make it available if the server is killed */
	tmp_errno = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error: ");
		exit(tmp_errno);
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
		exit(tmp_errno);
	}

	/* attempt to establish ourselves to listen for incomming conections */
	tmp_errno = listen(listen_fd, BACKLOG);

	if (tmp_errno == -1) {
		tmp_errno = errno;
		perror("Error: ");
		exit(tmp_errno);
	}

	/* begin loop to coninually check for client connections */
	length = sizeof(struct sockaddr_in);
	clients_connected = 0;
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

		clients_connected++;

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

		printf("%s %d\n", client_name, clients_connected);

		time_t current_time;
		time(&current_time);

		/* write date to connected client */
		if ( write(connection_fd, ctime(&current_time), DATE_SIZE) == -1) {
			tmp_errno = errno;
			perror("Error: ");
			exit(tmp_errno);
		}

		/* terminate connection with client */
		close(connection_fd);
		exit(0);
	}
}
