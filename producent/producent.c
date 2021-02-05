#include "producent.h"

int main(int argc, char* argv[])
{
    float tempo;
    char *ip_address = "localhost";
    unsigned short port;
//    int timer = timer_create(CLOCK_MONOTONIC, SIGEV_THREAD) ;
//    int rt = timer_create(CLOCK_REALTIME);
//    struct sigevent {
//
//    };

    if (argc != 4)
    {
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    get_arguments(&tempo, &ip_address, &port, argc, argv);

    printf("tempo: %g\nip address: %s\nport: %u\n", tempo, ip_address, port);

    int fd[2];
    char buf[640];

    if (pipe(fd) == -1)
    {
        perror("Failed to create pipe!");
        exit(EXIT_FAILURE);
    }

    switch (fork())
    {
        case -1:
            err(1,NULL);
        case  0:
            // TODO dystrybucja
            // dystrybucja wyciąganie z pipe'a i wysyłanie poprzez socket

            if (close(fd[1]) == -1)
            {
                perror("Failed to close write end of pipe!");
                exit(EXIT_FAILURE);
            }

//            if (read(fd[0], buf, sizeof(buf)) == -1)
//            {
//                perror("Failed to read from the pipe!");
//                exit(EXIT_FAILURE);
//            }

            while (read(fd[0], &buf, 1) > 0)
                write(STDOUT_FILENO, &buf, 1);



            exit(EXIT_SUCCESS);

        default:
            if (close(fd[0]) == -1)
            {
                perror("Failed to close read end of pipe!");
                exit(EXIT_FAILURE);
            }

//            if (timer_create() == -1)
//            {
//                perror("Failed to create timer!");
//                exit(EXIT_FAILURE);
//            }

            while (1)
            {
                for(int letter = 'A'; letter >= 'A' && letter <= 'z';)
                {
                    if (letter == 91) letter = 'a';
                    if (letter == 123) letter = 'A';

                    memset(buf, letter, sizeof(buf));
                    if (write_to_pipe(fd, buf, sizeof(buf), tempo)) letter++;
                }

                // configure_timer();
            }
            break;
    }

    return EXIT_SUCCESS;
}

void get_arguments(float *tempo, char **address, unsigned short *port, int argc, char *argv[])
{
    int opt;
    char *endptr;
    int index;
    
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                errno = 0;
                *tempo = strtof(optarg, &endptr);
                if ((errno == ERANGE && (*tempo == LONG_MAX || *tempo == LONG_MIN))
                    || (errno != 0 && *tempo == 0)) {
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
                fprintf(stderr, "Usage: ./producent -p <float> [<ip.addr>:]port\n");
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

int write_to_pipe(int *fd, char *buf, int size, float tempo)
{
    // TODO błędy do timestruct

    struct timespec ts;
    ts.tv_sec = 640 / tempo;
    ts.tv_nsec = ((float) 640 / tempo - 640 / tempo) * pow(10, 9);

    nanosleep(&ts, NULL);
    if (write(fd[1], buf, size) == -1)
    {
        perror("Failed to write into a pipe!");
        return 0;
    }

    return 1;
}
