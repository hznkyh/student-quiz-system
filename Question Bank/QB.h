#ifndef QB_H
#define QB_H

#define BUF_SIZE 1024
#define MAX_PAYLOAD_LEN 1024

struct message {
    int length; //Length of header
    char payload[MAX_PAYLOAD_LEN];
};
void create_socket(int *sockfd);
void bind_socket(int sockfd);
void listen_for_connections(int sockfd);
void handle_connection(int sockfd);
void close_connection(int connfd);
void get_local_ip(char *ip, int size);
char* getQuestions(char *unacceptableQuestions);
#endif
