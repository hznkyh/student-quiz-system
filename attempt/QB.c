#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buf[BUF_SIZE];

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9001);

    // Bind the socket to a specific address and port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else{
        printf("Binded to port 9001\n");
    }

    // Listen for incoming messages
    while (1) {
        int len = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (len < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        buf[len] = '\0';
        printf("Received message: %s\n", buf);

        // Send acknowledgement back to sender
        char ack_msg[BUF_SIZE];
        sprintf(ack_msg, "Acknowledgement for %s", buf);
        if (sendto(sockfd, ack_msg, strlen(ack_msg), 0, (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("sendto failed");
            exit(EXIT_FAILURE);
        }
    }

    // Close the socket
    close(sockfd);
    return 0;
}
