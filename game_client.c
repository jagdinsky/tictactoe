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
    ERR_CONNECT,
    ERR_FORK
};

int init_socket(const char *ip, int port) {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creating failed.");
        _exit(ERR_SOCKET);
    }
    struct hostent *host = gethostbyname(ip);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    memcpy(&server_address.sin_addr, host -> h_addr_list[0],
                            sizeof(server_address.sin_addr));
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    memcpy(&sin.sin_addr, host -> h_addr_list[0], sizeof(sin.sin_addr));
    int connect_status = connect(server_socket,
                (struct sockaddr *) &sin, sizeof(sin));
    if (connect_status < 0) {
        perror("Connection failed.");
        _exit(ERR_CONNECT);
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

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Incorrect arguments.");
        puts("./client <ip> <port>");
        puts("Example:");
        puts("./client 127.0.0.1 5000");
        return ERR_INCORRECT_ARGS;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int server = init_socket(ip, port);
    char example[9], field[9], flag, sign;
    read(server, &flag, 1);
    if (flag == '1')
        sign = 'X';
    else
        sign = 'O';
    int number;
    for (int i = 0; i < 9; i++)
        example[i] = '1' + i;
    print_field(example, 9);
    putchar('\n');
    while (1) {
        read(server, &flag, 1);
        if (flag == 'w') {
            printf("You won!\n");
            break;
        }
        if (flag == 'l') {
            printf("You lost.\n");
            break;
        }
        if (flag == 'g') {
            printf("Your turn:\n");
            read(server, field, 9);
            print_field(field, 9);
            scanf("%d", &number);
            field[number - 1] = sign;
            write(server, field, 9);
            print_field(field, 9);
        } else {
            printf("Opponent's turn...\n");
        }
    }
    return OK;
}
