#include "klient.h"

int main(int argc, char* argv[])
{
    unsigned int blocks;
    float consumption;
    float degradation;
    char *ip_address = "localhost";
    unsigned short port;
    struct sockaddr_in server_addr;
    char buf[WHOLE_MESSAGE_SIZE];
    // char buf[RECV_PACKET_SIZE];
    unsigned long long available_space;
    struct timespec timediff;

    char *report = "CLIENT REPORT:\n";


    get_arguments(argc, argv, &ip_address, &port, &blocks, &consumption, &degradation, &available_space);
    available_space = (unsigned int) blocks * BLOCK_SIZE;
    // available_space = BLOCK_SIZE-1;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    printf("%s:%d\n", ip_address, port);

    if (inet_aton(ip_address, &server_addr.sin_addr) == -1)
    {
        fprintf(stderr, "Failed to register address");
        exit(EXIT_FAILURE);
    }

    // printf("Available space before checking, if there's a space for message: %llu\n", available_space);
    if (clock_gettime(CLOCK_MONOTONIC, &timediff) == -1)
    {
        fprintf(stderr, "Failed to retrieve time of connection!");
        exit(EXIT_FAILURE);
    }

    long long unsigned a;
    while ((a = free_space(&available_space, degradation, &timediff, blocks)) >= WHOLE_MESSAGE_SIZE)
    {
        // report = realloc(report, )
        // printf("Client's free space: %llu\nDegradation rate: %f\n\n\n", a, degradation);
        int bytes_read = 0;
        struct timespec connection_time;
        struct timespec time_of_first_read;
        struct timespec disconnection_time;

        printf("Client's free space: %llu\n", a);

        int sock_fd = socket(AF_INET,SOCK_STREAM,0);
        if (sock_fd == -1)
        {
            perror("Failed to create socket");
            exit(EXIT_FAILURE);
        }

        if (connect(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
        {
            fprintf(stderr, "Connection to server failed!\n");
            exit(EXIT_FAILURE);
        }
        printf("CONNECTED\n");

        if (clock_gettime(CLOCK_MONOTONIC, &connection_time) == -1)
        {
            fprintf(stderr, "Failed to retrieve time of connection!");
            exit(EXIT_FAILURE);
        }

        while (bytes_read < WHOLE_MESSAGE_SIZE)
        {
            struct timespec delay;
            int bytes_read_in_iteration = 0;
            if ((bytes_read_in_iteration = recv(sock_fd, buf, WHOLE_MESSAGE_SIZE - bytes_read, MSG_DONTWAIT)) != -1)
            {
                if ((bytes_read == 0) && (bytes_read_in_iteration > 0))
                {
                    if (clock_gettime(CLOCK_MONOTONIC, &time_of_first_read) == -1)
                    {
                        fprintf(stderr, "Failed to retrieve time of first read!");
                        exit(EXIT_FAILURE);
                    }
                }
                bytes_read += bytes_read_in_iteration;

                printf("Available space: %llu\n", available_space);
                available_space -= bytes_read_in_iteration;
                printf("Bytes read: %d\nBytes read in iteration: %d\nAvailable space: %llu\n", bytes_read, bytes_read_in_iteration, available_space);

                delay.tv_sec = bytes_read_in_iteration / (int) consumption;
                delay.tv_nsec = (bytes_read_in_iteration / consumption - bytes_read_in_iteration / (int) consumption) * pow(10, 9);

                // printf("%s\n", buf);
                // printf("%d / %f = %f\n", bytes_read_in_iteration, consumption, bytes_read_in_iteration / consumption);
                printf("Delay: %ld.%ld\n", delay.tv_sec, delay.tv_nsec);

                nanosleep(&delay, NULL);
                printf("Available space: %llu\n\n\n", available_space);

                if (delay.tv_sec == 0) sleep(1);
            }
        }

        if (shutdown(sock_fd,SHUT_RDWR) == -1)
        {
            perror("Shutdown fail");
            exit(2);
        } else printf("DISCONNECTED!\n");

        if (clock_gettime(CLOCK_MONOTONIC, &disconnection_time) == -1)
        {
            fprintf(stderr, "Failed to retrieve time of disconnection!");
            exit(EXIT_FAILURE);
        }

        // generate_block_report(report, sock_fd, connection_time, time_of_first_read, disconnection_time);
    }

    generate_final_report();
    printf("%s", report);

    return 1;
}

void generate_final_report()
{
    struct timespec rt;
    if (clock_gettime(CLOCK_REALTIME, &rt) == -1)
    {
        perror("Unable to retrieve RealTime for final report");
        exit(EXIT_FAILURE);
    }

    printf("REALTIME: %ld.%ld\n", rt.tv_sec, rt.tv_nsec);
}

void generate_block_report(char *report, int sock_fd, struct timespec connection, struct timespec time_of_first_read, struct timespec disconnection)
{
    struct sockaddr_in connection_address;
    unsigned int address_len;
    struct timespec result;

    char *tmp = "";

    if (getsockname(sock_fd, (struct sockaddr*) &connection_address, &address_len) == -1)
    {
        fprintf(stderr, "Unable to obtain client's address");
        exit(EXIT_FAILURE);
    }

    printf("BLOCK REPORT:\n");
    printf("PID: %d\n", getpid());
    printf("Client's address: %s:%d\n", inet_ntoa(connection_address.sin_addr), (int) ntohs(connection_address.sin_port));

    if (timespec_subtract(&result, &time_of_first_read, &connection) == 1)
    {
        fprintf(stderr, "Time of first read - time of connection < 0\n");
        exit(EXIT_FAILURE);
    }

    // printf("Connection to first read: %ld.%ld\n", time_of_first_read.tv_sec - connection.tv_sec, time_of_first_read.tv_nsec - connection.tv_nsec);
    printf("Connection to first read: %ld.%ld\n", result.tv_sec, result.tv_nsec);

    if (timespec_subtract(&result, &disconnection, &time_of_first_read) == 1)
    {
        fprintf(stderr, "Time of disconnection - time of first read < 0\n");
        exit(EXIT_FAILURE);
    }

    // printf("First read to disconnection: %ld.%ld\n", disconnection.tv_sec - time_of_first_read.tv_sec, disconnection.tv_nsec - time_of_first_read.tv_nsec);
    printf("First read to disconnection: %ld.%ld\n", result.tv_sec, result.tv_nsec);

    // *report += tmp;
}


unsigned long long free_space(unsigned long long *available_space, float degradation_rate, struct timespec *time, unsigned int blocks)
{
    struct timespec last_read = *time;

    if (clock_gettime(CLOCK_MONOTONIC, time) == -1)
    {
        perror("Couldn't retrieve time!\n");
        exit(EXIT_FAILURE);
    }

    double time_diff = time->tv_sec - last_read.tv_sec + (time->tv_nsec - last_read.tv_nsec) / (double) 1000000000;

    printf("Free space before: %llu\n", *available_space);
    printf("Degradation rate * time diff: %f * %f\n", degradation_rate, time_diff);

    if ((*available_space + degradation_rate * time_diff) > (blocks * BLOCK_SIZE))
    {
        // printf("blocks * BLOCK_SIZE: %u * %d = %d\n", blocks, BLOCK_SIZE, blocks * BLOCK_SIZE);
        *available_space = blocks * BLOCK_SIZE;
    }
    else
    {
        // printf("blocks * BLOCK_SIZE: %u * %d = %d\n", blocks, BLOCK_SIZE, blocks * BLOCK_SIZE);
        *available_space += degradation_rate * time_diff;
    }


    printf("Free space after: %llu\n", *available_space);

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
                *blocks = (unsigned int) strtoul(optarg, NULL, 10);
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
                fprintf(stderr, "unknown getopt value\n");
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

int timespec_subtract (struct timespec *result, struct timespec *x, struct timespec *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_nsec < y->tv_nsec) {
        int nsec = (y->tv_nsec - x->tv_nsec) / pow(10, 9) + 1;
        y->tv_nsec -= pow(10, 9) * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_nsec - y->tv_nsec > 1000000) {
        int nsec = (x->tv_nsec - y->tv_nsec) / pow(10, 9);
        y->tv_nsec += pow(10, 9) * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_nsec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_nsec = x->tv_nsec - y->tv_nsec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}



//        int bytes_read = 0;
//        while (bytes_read < WHOLE_MESSAGE_SIZE)
//        {
//            if (recv(sock_fd, buf, WHOLE_MESSAGE_SIZE, MSG_DONTWAIT | MSG_PEEK) != -1)
//            {
//                bytes_read += recv(sock_fd, buf, WHOLE_MESSAGE_SIZE, MSG_DONTWAIT | MSG_PEEK);
//            }
//        }


//        if (recv(sock_fd, buf, WHOLE_MESSAGE_SIZE, MSG_DONTWAIT | MSG_PEEK) == WHOLE_MESSAGE_SIZE) // jeśli pod socketem nie ma 13kib to czekaj z tym odbieraniem pakietów
//        {
//            // printf("there's 13kib in server socket\n");
//            if (recv(sock_fd, buf, RECV_PACKET_SIZE, MSG_DONTWAIT) == RECV_PACKET_SIZE)
//            {
//                // printf("%s\n", buf);
//                available_space = available_space - RECV_PACKET_SIZE;
//                // write(STDOUT_FILENO, buf, sizeof(buf));
//                write(STDOUT_FILENO, buf, RECV_PACKET_SIZE);
//                printf("\n");
//            }
//            else
//            {
//                available_space = available_space - RECV_PACKET_SIZE;
//                // write(STDOUT_FILENO, buf, sizeof(buf));
//                write(STDOUT_FILENO, buf, RECV_PACKET_SIZE);
//                // printf("%s\nEnd of 13KiB message\n\n", buf);
//                printf("End of 13KiB message\n\n");
//            }
//        }
//        sleep(1);
//      generate_block_report(available_space);



//    printf("Degradation rate: %f\n", degradation_rate);
//    printf("timediff: %f\n", time_diff);
//    printf("%llu + %f * %f = %f\n", *available_space, degradation_rate, time_diff, *available_space + degradation_rate * time_diff);

//    if (degradation_rate * time_diff > *available_space)
//    {
//        *available_space = 0;
//        printf("A\n");
//    }
//    else if ((*available_space + degradation_rate * time_diff) > (blocks * BLOCK_SIZE))
//    {
//        *available_space = blocks * BLOCK_SIZE;
//        printf("B\n");
//    }
//    else
//    {
//        printf("C\n");
//        *available_space = *available_space + degradation_rate * time_diff;
//    }
