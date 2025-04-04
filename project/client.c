#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: client <hostname> <port> \n");
        exit(1);
    }

    // Only supports localhost as a hostname, but that's all we'll test on
    const char* addr =
        strcmp(argv[1], "localhost") == 0 ? "127.0.0.1" : argv[1];
    int port = atoi(argv[2]);

    //*Create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                 // use IPv4  use UDP

    //*Set stdin and socket nonblocking
    int flags = fcntl(sockfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    flags = fcntl(STDIN_FILENO, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flags);

    //*Construct server address
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET; // use IPv4
    serveraddr.sin_addr.s_addr = inet_addr(addr);

    // Set receiving port
    serveraddr.sin_port = htons(port); // Big endian

    int BUF_SIZE = 1024;

    char server_buf[BUF_SIZE];
    char client_buf[BUF_SIZE];

    // Listen loop
    while (1) {
        //*Receive from socket
        socklen_t serversize = sizeof(struct sockaddr_in);

        struct sockaddr_in recaddr;
        int bytes_recvd = recvfrom(sockfd, server_buf, BUF_SIZE, 
                        // socket  store data  how much
                           0, (struct sockaddr*) &recaddr, 
                           &serversize);

        //*If data, write to stdout
        if (bytes_recvd > 0) {
            write(1, server_buf, bytes_recvd);
        }

        //*Read from stdin
        int bytes_read = read(STDIN_FILENO, client_buf, sizeof(client_buf));

        if (bytes_read <= 0) continue;

        //*If data, send to socket
        int did_send = sendto(sockfd, client_buf, bytes_read,
                   // socket  send data   how much to send
                      0, (struct sockaddr*) &serveraddr,
                   // flags   where to send
                      sizeof(serveraddr));

        if (did_send < 0) return -1;
    }

    return 0;
}