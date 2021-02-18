#include "producent.h"
// http://ptgmedia.pearsoncmg.com/images/0321335724/samplechapter/seacord_ch05.pdf

int main(int argc, char* argv[])
{
    float tempo;
    char *ip_address = "localhost";
    unsigned short port;

//    timer_t timer_rt;
//    struct sigevent sevp;
//    sevp.sigev_notify = SIGEV_NONE;
//    if (timer_create(CLOCK_REALTIME, &sevp, &timer_rt) == -1)
//    {
//        perror("Failed to create realtime clock!");
//        exit(EXIT_FAILURE);
//    }

    int fd[2];
    char buf[WRITE_PACKET_SIZE];

    int epollfd;
    int serverfd;
    struct epoll_event event, evlist[MAX_CLIENTS + 1];

    if (argc != 4)
    {
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    get_arguments(&tempo, &ip_address, &port, argc, argv);

    // printf("tempo: %g\nip address: %s\nport: %u\n", tempo, ip_address, port);

    if (pipe2(fd, O_NONBLOCK) == -1)
    {
        perror("Failed to create pipe!");
        exit(EXIT_FAILURE);
    }

    switch (fork())
    {
        case -1:
            err(1,NULL);
        case  0:
            if (close(fd[1]) == -1)
            {
                perror("Failed to close write end of pipe!");
                exit(EXIT_FAILURE);
            }

            if ((serverfd = create_server(ip_address, port)) == -1)
            {
                perror("Failed to create server");
                exit(EXIT_FAILURE);
            }

            if ((epollfd = create_epoll_instance(serverfd)) == -1)
            {
                perror("Failed to create epoll instance");
                exit(EXIT_FAILURE);
            }

            char (*locked_resources)[WHOLE_MESSAGE_SIZE];

            while (1)
            {
                int open_fds = epoll_wait(epollfd, evlist, MAX_CLIENTS + 1, 0);

                // char locked_resources[open_fds][WHOLE_MESSAGE_SIZE]; ŹLE
                // printf("events in this loop iteration: %d\n", open_fds);

                sleep(1);
                for (int i = 0; i < open_fds; i++)
                {
//                    printf("  fd=%d; events: %s%s%s%s\n", evlist[i].data.fd,
//                           (evlist[i].events & EPOLLIN) ? "EPOLLIN " : "",
//                           (evlist[i].events & EPOLLOUT) ? "EPOLLOUT " : "",
//                           (evlist[i].events & EPOLLHUP) ? "EPOLLHUP " : "",
//                           (evlist[i].events & EPOLLERR) ? "EPOLLERR " : "");

                    if (evlist[i].data.fd == serverfd)
                    {
                        int client_fd;
                        if ((client_fd = polaczenie(serverfd)) != -1)
                        {
                            event.events = EPOLLOUT;
                            event.data.fd = client_fd;
                            printf("Connection accepted: fd=%d\n", client_fd);

                            makeSocketNonBlocking(client_fd);

                            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                            {
                                perror("Failed to create epoll event for connected socket");
                                exit(EXIT_FAILURE);
                            }

                            // char buffer[WHOLE_MESSAGE_SIZE];
                            int bytes_read = 0;

                            while (bytes_read < WHOLE_MESSAGE_SIZE)
                            {
                                bytes_read += read(fd[0], locked_resources[i], WHOLE_MESSAGE_SIZE - bytes_read);
                                printf("%d\n", bytes_read);
                            }
                        }
                    }
                    else if (evlist[i].events & EPOLLOUT)
                    {
                        int bytes_sent;
//                        if ((bytes_sent = send(evlist[i].data.fd, locked_resources[i], SEND_PACKET_SIZE, 0)) == -1)
//                        {
//                            perror("Failed to reserve data for client (sending data to server failed)");
//                            exit(EXIT_FAILURE);
//                        }
                        // printf("%s\n", locked_resources[i]);

                        if ((bytes_sent = send(evlist[i].data.fd, , SEND_PACKET_SIZE, 0)) == -1)
                        {
                            perror("Failed to reserve data for client (sending data to server failed)");
                            exit(EXIT_FAILURE);
                        }

                        // locked_resources = locked_resources + bytes_sent;
                        printf("%d bytes sent to client %d\n", bytes_sent, evlist[i].data.fd);
                    }
                    else if ((evlist[i].events & EPOLLERR) || (evlist[i].events & EPOLLHUP))
                    {
                        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, evlist[i].data.fd, &event) == -1)
                        {
                            perror("Unable to delete epoll event");
                            exit(EXIT_FAILURE);
                        }
                        if (close(evlist[i].data.fd) == -1)
                        {
                            perror("Unable to close client's descriptor");
                            exit(EXIT_FAILURE);
                        }
                    }
                    // if (close(evlist[i].data.fd) == -1)
                    // sleep(1);
                }
            }

            exit(EXIT_SUCCESS);

        default:
            if (close(fd[0]) == -1)
            {
                perror("Failed to close read end of pipe");
                exit(EXIT_FAILURE);
            }

            while (1)
            {
                for(int letter = 'A'; letter >= 'A' && letter <= 'z';)
                {
                    if (letter == 91) letter = 'a';
                    if (letter == 123) letter = 'A';

                    memset(buf, letter, sizeof(buf));
                    if (write_to_pipe(fd, buf, tempo)) letter++;
                }
            }
            break;
    }

    return EXIT_SUCCESS;
}

int polaczenie( int sockfd ) {
    struct sockaddr_in peer;
    socklen_t addr_len = sizeof(peer);

    int new_socket = accept(sockfd,(struct sockaddr *)&peer,&addr_len);
    if( new_socket != -1 )
    {
        fprintf(stderr,"Nawiązane połączenie z klientem %s (port %d)\n",
                inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
    }
    return new_socket;
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

int write_to_pipe(int *fd, char *buf, float tempo)
{
    struct timespec ts;
    ts.tv_sec = WRITE_PACKET_SIZE / tempo;
    ts.tv_nsec = (WRITE_PACKET_SIZE / tempo - WRITE_PACKET_SIZE / (int) tempo) * pow(10, 9);

    nanosleep(&ts, NULL);
    if (write(fd[1], buf, WRITE_PACKET_SIZE) != WRITE_PACKET_SIZE) return 0;
//    long c;
//    if ((c = write(fd[1], buf, WRITE_PACKET_SIZE)) != WRITE_PACKET_SIZE) return 0;
//    printf("%ld\n", c);

    return 1;
}

int create_server(char *ip_address, int port)
{
    int server_fd;
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,0)) == -1)
    {
        perror("Failed to create server socket!");
        return -1;
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Failed to bind\n");
        return -1;
    }

    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        perror("Failed to listen on server socket\n");
        return -1;
    }

    printf("Listening...\n");

    makeSocketNonBlocking(server_fd);

    return server_fd;
}

void makeSocketNonBlocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, NULL);

    if(flags == -1)
    {
        perror("Failed to set up socket flags");
        exit(EXIT_FAILURE);
    }

    flags |= O_NONBLOCK;

    if(fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("Failed to socket descriptor non blocking");
        exit(EXIT_FAILURE);
    }
}

int create_epoll_instance(int server_fd)
{
    int epollfd;
    struct epoll_event server_listen_event;

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("Failed to create epoll instance");
        return -1;
    }

    server_listen_event.events = EPOLLIN;
    server_listen_event.data.fd = server_fd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &server_listen_event) == -1)
    {
        perror("Failed to set up event for production");
        return -1;
    }

    return epollfd;
}
