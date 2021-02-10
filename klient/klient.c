#include "klient.h"

int main(int argc, char* argv[])
{
    unsigned int blocks;
    float consumption;
    float degradation;
    char *ip_address = "localhost";
    unsigned short port;
    struct sockaddr_in server_addr;
    char buf[RECV_PACKET_SIZE];
    unsigned long long available_space;

    struct timespec connection_time;
    char *report = "CLIENT REPORT:\n";

    int sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if (sock_fd == -1)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    get_arguments(argc, argv, &ip_address, &port, &blocks, &consumption, &degradation, &available_space);
    available_space = (unsigned int) blocks * BLOCK_SIZE;
    available_space = BLOCK_SIZE-1;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    printf("%s:%d\n", ip_address, port);

    if (inet_aton(ip_address, &server_addr.sin_addr) == -1)
    {
        fprintf(stderr, "Failed to register address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
    {
        fprintf(stderr, "Connection to server failed!\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected!\n");

    if (clock_gettime(CLOCK_MONOTONIC, &connection_time) == -1)
    {
        fprintf(stderr, "Failed to retrieve time of connection!");
        exit(EXIT_FAILURE);
    }
    printf("%llu\n", available_space);

    while (free_space(&available_space, consumption, &connection_time, blocks) >= WHOLE_MESSAGE_SIZE)
    {
        // report = realloc(report, )
        if (recv(sock_fd, &buf, WHOLE_MESSAGE_SIZE, MSG_DONTWAIT | MSG_PEEK) == WHOLE_MESSAGE_SIZE)
        {
            printf("there's 13kib in server socket");
            if (recv(sock_fd, &buf, RECV_PACKET_SIZE, MSG_DONTWAIT | MSG_WAITALL) == WHOLE_MESSAGE_SIZE)
            {
                available_space = available_space - RECV_PACKET_SIZE;
                write(STDOUT_FILENO, &buf, sizeof(buf));
            }
        }
        sleep(1);
        // generate_block_report(available_space);
    }

    if( shutdown(sock_fd,SHUT_RDWR) ) {
        perror("Shutdown fail\n");
        exit(2);
    } else {
        generate_final_report();
        printf("%s", report);
    }

    return 1;
}

void generate_final_report()
{
    struct timespec rt;
    if (clock_gettime(CLOCK_REALTIME, &rt) == -1)
    {
        perror("Unable to retrieve RealTime for final report!");
        exit(EXIT_FAILURE);
    }

    printf("REALTIME: %ld.%ld\n", rt.tv_sec, rt.tv_nsec);
    printf("PID: %d\n", getpid());
}

//void generate_block_report(unsigned long long available_space)
//{
//    // printf("REALTIME: %d", timer_gettime(realtime, ));
//    printf("BLOCK REPORT:\n");
//    printf("%llu\n", available_space);
//    sleep(1);
//    // printf("THE END\n");
//}

unsigned long long free_space(unsigned long long *available_space, int degradation_rate, struct timespec *time, unsigned int blocks)
{
    struct timespec last_read = *time;

    if (clock_gettime(CLOCK_MONOTONIC, time) == -1)
    {
        perror("Couldn't retrieve time!\n");
        exit(EXIT_FAILURE);
    }

    double time_diff = time->tv_sec - last_read.tv_sec + (time->tv_nsec - last_read.tv_nsec) / (double) 1000000000;

    if (degradation_rate * time_diff > *available_space) *available_space = 0;
    else if (*available_space + degradation_rate * time_diff > blocks * BLOCK_SIZE) *available_space = blocks * BLOCK_SIZE;
    else *available_space = *available_space + degradation_rate * time_diff;

    return *available_space;
}

void get_arguments(int argc, char* argv[], char **address, unsigned short *port, unsigned int *blocks, float *consumption, float *degradation, unsigned long long *available_space)
{
    int opt;
    char *endptr;
    int index;

    while ((opt = getopt(argc, argv, "c:p:d:")) != -1)
    {
        switch (opt)
        {
            case 'c':
                *blocks = (int) strtoul(optarg, NULL, 10);
                break;
            case 'd':
                errno = 0;
                *degradation = strtof(optarg, &endptr);
                if ((errno == ERANGE && (*degradation == LONG_MAX || *degradation == LONG_MIN))
                    || (errno != 0 && *degradation == 0)) {
                    perror("strtof");
                    exit(EXIT_FAILURE);
                }
                if (endptr == optarg || *endptr != '\0')
                {
                    fprintf(stderr, "Parameter d has to be a float! %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                errno = 0;
                *consumption = strtof(optarg, &endptr);
                if ((errno == ERANGE && (*consumption == LONG_MAX || *consumption == LONG_MIN))
                    || (errno != 0 && *consumption == 0)) {
                    perror("strtof");
                    exit(EXIT_FAILURE);
                }
                if (endptr == optarg || *endptr != '\0')
                {
                    fprintf(stderr, "Parameter p has to be a float! %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "i dont fucking care anymore");
                exit(EXIT_FAILURE);
        }
    }

    for (index = optind; index < argc; index++)
    {
        if (is_argument_valid(argv[index], address, port)) return;
        exit(EXIT_FAILURE);
    }
}

int is_argument_valid(char *argv, char **ip_address, unsigned short *port)
{
    char *token;
    char *saveptr;
    int amount_of_colons = 0;

    for (int i = 0; amount_of_colons < 2 && i<=strlen(argv); i++) if (argv[i] == ':') amount_of_colons++;

    switch (amount_of_colons)
    {
        case 0:
            if (is_port(argv))
            {
                *port = (unsigned short) strtoul(argv, NULL, 10);
            }
            break;
        case 1:
            token = strtok_r(argv, ":", &saveptr);
            if (!is_ip_address(token))
            {
                fprintf(stderr, "Incorrect IP address!\n");
                exit(EXIT_FAILURE);
            }
            *ip_address = token;
            if (**(ip_address) == '.') *ip_address = *ip_address + 1;
            if (*(*ip_address + strlen(*ip_address) - 1) == '.') *(*ip_address + strlen(*ip_address) - 1) = '\0';

            token = strtok_r(NULL, ":", &saveptr);
            if (!is_port(token))
            {
                fprintf(stderr, "Incorrect port!\n");
                exit(EXIT_FAILURE);
            }
            *port = strtol(token, NULL, 10);

            break;
        default:
            fprintf(stderr, "Given IP address is incorrect.\n");
            return 0;
    }
    return 1;
}

int is_ip_address(char *argv)
{
    char* ip = malloc(sizeof(argv));
    int num, dots = 0;
    char *ptr;
    strcpy(ip, argv);
    if (ip == NULL)
        return 0;
    ptr = strtok(ip, ".");
    if (ptr == NULL)
        return 0;
    while (ptr) {
        if (!validate_number(ptr))
            return 0;
        num = atoi(ptr);
        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, ".");
            if (ptr != NULL)
                dots++;
        } else
            return 0;
    }
    if (dots != 3)
        return 0;
    return 1;
}

int validate_number(char *str)
{
    while (*str) {
        if(!isdigit(*str)){
            return 0;
        }
        str++;
    }
    return 1;
}

int is_port(char *argv)
{
    if (strtoul(argv, NULL, 10) < 0 || strtoul(argv, NULL, 10) > 65535)
    {
        fprintf(stderr, "Incorrect input. Port is outside of range 0-65535.\n");
        exit(EXIT_FAILURE);
    }
    return 1;
}
