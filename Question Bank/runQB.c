#include "QB.h"
#include <stdio.h>

int main(void){
    int sockfd;

    // Create a socket
    create_socket(&sockfd);

    // Bind the socket to a specific address and port
    bind_socket(sockfd);

    // Listen for incoming connections


    // Accept incoming connections and handle them
    while (1) {
        listen_for_connections(sockfd);
        handle_connection(sockfd);
    }
    
    // Close the socket
    close_connection(sockfd);

    return 0;
}
