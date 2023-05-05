#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[]){
    int sockfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buf[BUF_SIZE];

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9002); // bind to port 9002

    // Bind the socket to a specific address and port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else{
        printf("Binded to port 9002\n"); // 9001 doesnt work wtf?
    }

    // Listen for incoming connections, need to send acknowledgement back when failing
    if (listen(sockfd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connection
    connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (connfd < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    // Use read to read message recieved
    int len = read(connfd, buf, BUF_SIZE);
    if (len < 0) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }
    buf[len] = '\0';
    printf("Received message: %s\n", buf);
     // From here, QB can read a message request to TM

    // Basic ACK, needs to acknowledge recieved and failed messages
    char ack_msg[BUF_SIZE];
    sprintf(ack_msg, "Acknowledgement for %s", buf);
    if (send(connfd, ack_msg, strlen(ack_msg), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

    // Close socket connection
    close(connfd);
    close(sockfd);
    return 0;
}
// Needs modularisation
