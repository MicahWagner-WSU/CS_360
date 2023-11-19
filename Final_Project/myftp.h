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
#include <limits.h>

#define MY_PORT_NUMBER 49999
#define BACKLOG 4
#define READ_BUF_LEN 1024
#define MAX_ARG_LENGTH 4 + PATH_MAX