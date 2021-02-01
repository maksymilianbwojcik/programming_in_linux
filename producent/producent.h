#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

void get_arguments(float *tempo, char **address, unsigned short *port, int argc, char* argv[]);
int is_argument_valid(char *argv, char *ip_address, unsigned short *port);
int is_ip_address(char *argv);
int is_port(char *argv);
int validate_number(char *str);
