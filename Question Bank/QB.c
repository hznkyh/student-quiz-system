#include "QB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>


int calculate_checksum(char* buf, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += buf[i];
    }
    return sum;
}

// Not DHCP, i doubt thats a requirement?
// based on this https://www.sanfoundry.com/c-program-get-ip-address/

void get_local_ip(char *ip, int size) {
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *sa;
    char host[NI_MAXHOST];
    int found = 0;

    // Get list of network interfaces and their associated addresses
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    // Iterate over list of network interfaces
    for (ifa = ifaddr; ifa != NULL && !found; ifa = ifa->ifa_next) {
        // Retriee IPV4 addredd
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET) {
            // Make sure that it doesnt use local host
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            // Converts to array 
            if (strcmp(inet_ntoa(sa->sin_addr), "127.0.0.1") != 0) {
               // Host name ip and address to buffer
                if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
                    strncpy(ip, host, size); // store iP, host and size
                    found = 1; // On successful find
                }
            }
        }
    }

    freeifaddrs(ifaddr); // Free ifaddr memory
    // If it fails to find the computer's IP address
    if (!found) {
        fprintf(stderr, "Unable to find local IP address\n");
        exit(EXIT_FAILURE);
    }
}

void create_socket(int *sockfd) {
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
}

void bind_socket(int sockfd) {
    struct sockaddr_in server_addr;
    char ip[NI_MAXHOST];
    int port = 9001;

    // Get local IP address
    get_local_ip(ip, sizeof(ip));
    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    // Bind the socket to a specific address and port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else{
        printf("Server running on %s:%d\n", ip, port);
    }
}

void listen_for_connections(int sockfd) {
    if (listen(sockfd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
}

void handle_connection(int sockfd) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fail = 0;
    int connfd;
    printf("* Ready to accept()\n");
    // Accept the connection
    if ((connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("* Transmission accepted\n");

    // Read the message and size
    struct message msg;
    ssize_t n;
    int payload_len;

    //Receive the Message header
    n = recv(connfd, &msg, sizeof(msg), 0);
        if (n <= 0) {
            perror("recv");
            exit(1);
        }
    msg.length = ntohl(msg.length);

    printf("Received message of length %d: '%s' \n", msg.length, msg.payload);

    char *source = msg.payload;
    char header[msg.length];
    for (int i = 0; i < msg.length; i++) {
        strncat(header, &source[i], 1);
    }
    char *newPayload = msg.payload+msg.length;
    
    printf("HEADER: %s\nPAYLOAD: %s\n",header, newPayload);
    
    char* question;
    
    if ( strcmp(header, "questions")){
        question = getQuestions(newPayload);
        
    }
    

    // Basic ACK, needs to acknowledge received and failed messages
    char ack_msg[BUF_SIZE];
    sprintf(ack_msg, "QB ACK");
    if (send(connfd, ack_msg, strlen(ack_msg), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

}

// The parameter is a list of questions that they can't be served because
// the student has already been served it.
char* getQuestions(char *unacceptableQuestions){
    if(unacceptableQuestions == NULL){
        //if we can pick any question then pick a random one.

    }
    else
    {
        //else pick a valid question.

    }
}

void close_connection(int connfd) {
    close(connfd);
}

int main(void){
    int sockfd;

    create_socket(&sockfd);

    bind_socket(sockfd);

    // Listen for incoming connections and handle them
    while (1) {
        listen_for_connections(sockfd);
        handle_connection(sockfd);
    }
    
    // Close the socket
    close_connection(sockfd);

    return 0;
}