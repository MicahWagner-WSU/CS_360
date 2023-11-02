#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int run_server();

int run_client(char *hostname);

int main(int argc, char const *argv[]){

	if (argc > 1) {
		if (strcmp(argv[1], "client") == 0) {
			printf("client");
		} else if (strcmp(argv[1], "server") == 0){
			printf("server");
		} else {
			printf("Not proper arg");
		}
	} else {
		printf("not enough args");
	}
    return 0;
}

int run_client(char *hostname) {

}

int run_server() {
	
}

