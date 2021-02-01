#include "producent.h"

int main(int argc, char* argv[])
{
    float tempo;
    char *ip_address = "localhost";
    unsigned short port;

    if (argc != 4)
    {
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    get_arguments(&tempo, &ip_address, &port, argc, argv);

    printf("tempo: %g\nip address: %s\n", tempo, ip_address);

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
                fprintf(stderr, "Usage: ./producent -p <float> [<ip.addr>:]port");
        }
    }

    for (index = optind; index < argc; index++)
    {
        if (is_argument_valid(argv[index], *address, port))
        {
            // TODO
            *address = argv[index];
            return;
        }
        exit(EXIT_FAILURE);
    }
}

int is_argument_valid(char *argv, char *ip_address, unsigned short *port)
{
    int amount_of_colons = 0;
    for (int i = 0; amount_of_colons < 2 && i<=strlen(argv); i++) if (argv[i] == ':') amount_of_colons++;
    switch (amount_of_colons)
    {
        case 0:
            if (is_port(argv))
            {
                port = strtol(argv, );
            }
            break;
        case 1:

            break;
        default:
            fprintf(stderr, "Given IP address is incorrect.\n");
            return 0;
    }
    return 1;
}

int is_ip_address(char *ip)
{
    int num, dots = 0;
    char *ptr;
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
    return 1;
}
