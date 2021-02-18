#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <math.h>

#define RECV_PACKET_SIZE 4096
#define WHOLE_MESSAGE_SIZE 13312
#define BLOCK_SIZE 30720

void get_arguments(int argc, char* argv[], char **address, unsigned short *port, unsigned int *blocks, float *consumption, float *degradation, unsigned long long *available_space);
int is_argument_valid(char *argv, char **ip_address, unsigned short *port);
int is_ip_address(char *argv); // zdecydowanie nie jest to idiotoodporna funkcja
int is_port(char *argv);
int validate_number(char *str);
unsigned long long free_space(unsigned long long *available_space, float degradation_rate, struct timespec *time, unsigned int blocks);
void generate_final_report();
void generate_block_report(char *report, int sockfd, struct timespec connection, struct timespec time_of_first_read, struct timespec disconnection);
int timespec_subtract (struct timespec *result, struct timespec *x, struct timespec *y);
