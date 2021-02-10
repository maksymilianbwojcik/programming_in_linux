#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <err.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/sendfile.h>

#define PIPE_SIZE fcntl(fd[0], F_GETPIPE_SZ)
#define MAX_CLIENTS 5
// #define MAX_CLIENTS 999
#define WRITE_PACKET_SIZE 640

void get_arguments(float *tempo, char **address, unsigned short *port, int argc, char* argv[]);
int is_argument_valid(char *argv, char **ip_address, unsigned short *port);
int is_ip_address(char *argv); // zdecydowanie nie jest to idiotoodporna funkcja
int is_port(char *argv);
int validate_number(char *str);

int write_to_pipe(int *fd, char *buf, float tempo);

int create_epoll_instance(int server_fd);
int create_server(char *ip_address, int port);

void makeSocketNonBlocking(int fd); // chyba nadmiarowe, skoro są flagi do send i recv
void create_pipe();

int handle_request(int fd);

int polaczenie( int sockfd );
