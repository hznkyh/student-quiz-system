#ifndef QB_H
#define QB_H

#define BUF_SIZE 1024

void create_socket(int *sockfd);
void bind_socket(int sockfd);
void listen_for_connections(int sockfd);
void handle_connection(int sockfd);
void close_connection(int connfd);

#endif
