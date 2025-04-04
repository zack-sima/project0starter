#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: server <port>\n");
        exit(1);
    }

    int port = atoi(argv[1]);

    // *Create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // use IPv4  use UDP

    // *Set stdin and socket nonblocking
    int flags = fcntl(sockfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    flags = fcntl(STDIN_FILENO, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flags);

    // *Construct server address
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET; // use IPv4

    // accept all connections, same as inet_addr("0.0.0.0") 
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Set receiving port
    servaddr.sin_port = htons(port); // Big endian
    
    // *Bind address to socket
    int did_bind = bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

    // Error if did_bind < 0 :(
    if (did_bind < 0) return -1;
    
    //*Create sockaddr_in and socklen_t buffers to store client address
    struct sockaddr_in clientaddr = {0}; // temporary client address storage
    socklen_t clientsize = sizeof(clientaddr);
    
    // NEW: Create a separate variable to hold the connected client's address
    int client_connected = 0;
    struct sockaddr_in connected_client = {0};

    int BUF_SIZE = 1024;
    char client_buf[BUF_SIZE];
    char server_buf[BUF_SIZE];

    //*Listen loop
    while (1) {
        //*Receive from socket
        int bytes_recvd = recvfrom(sockfd, client_buf, BUF_SIZE, 
                           0, (struct sockaddr*) &clientaddr, 
                           &clientsize);

        //*If no data, continue
        if (bytes_recvd <= 0) continue;

        //*Write received data to stdout
        write(STDOUT_FILENO, client_buf, bytes_recvd);

        // NEW: Save the client's address if not already connected
        if (!client_connected) {
            client_connected = 1;
            memcpy(&connected_client, &clientaddr, sizeof(clientaddr));
        }

        //*Read from stdin (directly read, not fgets)
        int bytes_read = read(STDIN_FILENO, server_buf, sizeof(server_buf));

        if (bytes_read <= 0) continue;

        int did_send = sendto(sockfd, server_buf, bytes_read, 
                              0, (struct sockaddr*) &connected_client, 
                              sizeof(connected_client));

        if (did_send < 0) return -1;
    }

    return 0;
}
