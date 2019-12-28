#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <signal.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN,
    ERR_FORK
};

int init_socket(int port) {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creating failed.");
        _exit(ERR_SOCKET);
    }
    int socket_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                            &socket_option, sizeof(socket_option));
    if (server_socket < 0) {
        perror("Setting socket options failed.");
        _exit(ERR_SETSOCKETOPT);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int bind_status = bind(server_socket, (struct sockaddr *) &server_address,
                                                    sizeof(server_address));
    if (bind_status < 0) {
        perror("Binding socket address failed.");
        _exit(ERR_BIND);
    }
    int listen_status = listen(server_socket, 4);
    if (listen_status < 0) {
        perror("Bind socket address failed.");
        _exit(ERR_LISTEN);
    }
    return server_socket;
}

void print_field(char *field, int n) {
    for (int i = 0; i < n; i++) {
        putchar('|');
        putchar(field[i]);
        putchar('|');
        if (i % 3 == 2)
            putchar('\n');
    }
}

int ready_field(char *field) {
    if (field[0] == field[2] && field[0] == field[3]) {
        if (field[0] == 'X')
            return 1;
        if (field[0] == 'O')
            return 2;
    }
    if (field[3] == field[4] && field[3] == field[5]) {
        if (field[3] == 'X')
            return 1;
        if (field[3] == 'O')
            return 2;
    }
    if (field[6] == field[7] && field[6] == field[8]) {
        if (field[6] == 'X')
            return 1;
        if (field[6] == 'O')
            return 2;
    }
    if (field[0] == field[3] && field[0] == field[6]) {
        if (field[0] == 'X')
            return 1;
        if (field[0] == 'O')
            return 2;
    }
    if (field[1] == field[4] && field[1] == field[7]) {
        if (field[1] == 'X')
            return 1;
        if (field[1] == 'O')
            return 2;
    }
    if (field[2] == field[5] && field[2] == field[8]) {
        if (field[2] == 'X')
            return 1;
        if (field[2] == 'O')
            return 2;
    }
    if (field[0] == field[4] && field[0] == field[8]) {
        if (field[0] == 'X')
            return 1;
        if (field[0] == 'O')
            return 2;
    }
    if (field[2] == field[4] && field[2] == field[6]) {
        if (field[2] == 'X')
            return 1;
        if (field[2] == 'O')
            return 2;
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("Incorrect arguments.");
        puts("./server <port>");
        puts("Example:");
        puts("./server 5000");
        return ERR_INCORRECT_ARGS;
    }
    int port = atoi(argv[1]);
    int server_socket = init_socket(port);
    struct sockaddr_in client_address[2];
    socklen_t size[2];
    int client_socket[2];
    for (int i = 0; i < 2; i++) {
        client_socket[i] = accept(server_socket,
                            (struct sockaddr *) &client_address[i], &size[i]);
        printf("Player %d: %s %d\n", i + 1,
                                        inet_ntoa(client_address[i].sin_addr),
                                            ntohs(client_address[i].sin_port));
    }
    char one = '1', two = '2';
    write(client_socket[0], &one, 1);
    write(client_socket[1], &two, 1);
    char field[9], stop = 's', go = 'g', won = 'w', lost = 'l';
    int result;
    for (int i = 0; i < 9; i++)
        field[i] = ' ';
    for (int i = 0; ; i++, i %= 2) {
        write(client_socket[i], &go, 1);
        write(client_socket[1 - i], &stop, 1);
        write(client_socket[i], field, 9);
        read(client_socket[i], field, 9);
        result = ready_field(field);
        if (result == 1) {
            write(client_socket[0], &won, 1);
            write(client_socket[1], &lost, 1);
            break;
        }
        if (result == 2) {
            write(client_socket[0], &won, 1);
            write(client_socket[1], &lost, 1);
            break;
        }
    }
    return OK;
}
