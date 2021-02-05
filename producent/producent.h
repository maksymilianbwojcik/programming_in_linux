#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <err.h>
#include <time.h>
#include <math.h>
#include <signal.h>

#define PIPE_SIZE 65536

void get_arguments(float *tempo, char **address, unsigned short *port, int argc, char* argv[]);
int is_argument_valid(char *argv, char **ip_address, unsigned short *port);
int is_ip_address(char *argv); // zdecydowanie nie jest to idiotoodporna funkcja
int is_port(char *argv);
int validate_number(char *str);
int write_to_pipe(int *fd, char *buf, int size, float tempo);

// DataBlock create_data_block(char letter, float tempo);
