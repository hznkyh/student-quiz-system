#ifndef QB_H
#define QB_H

#define BUF_SIZE 1024
#define MAX_PAYLOAD_LEN 1024

struct message {
    //int type;      // Type of message (e.g. 0 = login, 1 = logout, 2 = send message)
    int length;    // Length of payload in bytes
    char payload[MAX_PAYLOAD_LEN];  // Payload data
};
void create_socket(int *sockfd);
void bind_socket(int sockfd);
void listen_for_connections(int sockfd);
void handle_connection(int sockfd);
void close_connection(int connfd);
void get_local_ip(char *ip, int size);
#endif
