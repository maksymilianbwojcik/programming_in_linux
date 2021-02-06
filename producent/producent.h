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

// #define PIPE_SIZE 65536
#define PIPE_SIZE fcntl(fd[0], F_GETPIPE_SZ)
#define MAX_CLIENTS 5
// #define MAX_CLIENTS 999
#define WRITE_PACKET_SIZE 640
#define RECV_PACKET_SIZE 4096
#define WHOLE_MESSAGE_SIZE 13312

void get_arguments(float *tempo, char **address, unsigned short *port, int argc, char* argv[]);
int is_argument_valid(char *argv, char **ip_address, unsigned short *port);
int is_ip_address(char *argv); // zdecydowanie nie jest to idiotoodporna funkcja
int is_port(char *argv);
int validate_number(char *str);

int write_to_pipe(int *fd, char *buf, int size, float tempo);

void create_epoll_instance();
void set_up_epoll_descriptors();

void create_pipe();



// DataBlock create_data_block(char letter, float tempo);
