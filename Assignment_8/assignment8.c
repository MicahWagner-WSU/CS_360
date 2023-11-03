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

#define MY_PORT_NUMBER 49999

void run_server();

void run_client(const char *hostname);

int main(int argc, char const *argv[]){

	if (argc > 1) {
		if (strcmp(argv[1], "client") == 0) {
			run_client(argv[2]);
		} else if (strcmp(argv[1], "server") == 0){
			run_server();
		} else {
			printf("Not proper arg");
		}
	} else {
		printf("not enough args");
	}
    return 0;
}

void run_client(const char *hostname) {
	int socket_fd;
	struct addrinfo hints, *actual_data;
	char port_string[NI_MAXSERV] = { 0 };
	
	memset(&hints, 0, sizeof(hints));
	int err;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	sprintf(port_string, "%d", MY_PORT_NUMBER);

	err = getaddrinfo(hostname, port_string, &hints, &actual_data);

	socket_fd = socket(actual_data->ai_family, actual_data->ai_socktype, 0);
	connect(socket_fd, actual_data->ai_addr, actual_data->ai_addrlen);

}

void run_server() {
	int listen_fd, connection_fd, length;
	struct sockaddr_in server_address, client_address;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(MY_PORT_NUMBER);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listen_fd, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in));

	listen(listen_fd, 1);

	length = sizeof(struct sockaddr_in);
	for(;;) {
		while(waitpid(-1, NULL, WNOHANG) > 0);

		connection_fd = accept(listen_fd, (struct sockaddr *) &client_address, (socklen_t *) &length);

		if(fork()) {
			close(connection_fd);
			continue;
		}

		char client_name[NI_MAXHOST];
		int client_entry;

		client_entry = getnameinfo(
			(struct sockaddr *) &client_address, 
			sizeof(struct sockaddr_in),
			client_name,
			sizeof(client_name),
			NULL,
			0,
			NI_NUMERICSERV);

		printf("%s %d\n", client_name, );
	}
}
