#include "QB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <time.h>

//gcc QB.c -o QB 

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
    memmove(header, header+1, strlen(header)); //removes the non-printable STX control character.
    char *newPayload = msg.payload+msg.length;
    
    printf("HEADER: '%s'\nPAYLOAD: '%s'\n",header, newPayload);
    
    // Basic ACK, needs to acknowledge received and failed messages
    char ack_msg[BUF_SIZE];
    sprintf(ack_msg, "QB ACK\n");
    if (send(connfd, ack_msg, strlen(ack_msg), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

    Question* questions;
    
    if (strcmp(header, "question") == 0){
        printf("TM requested Questions...\n");
        questions = read_questions_file();
        send_questions(questions, connfd);
    }else if(strcmp(header, "answer") == 0){
        printf("Marking Questions feature not built yet\n");
    }
    else{
        printf("ERROR: Header '%s' not recognised.\n",header);
    }

}

//Used to randomly generate a number within the range of the number of questions we have. Ensures no duplicates.
int* generate_questions_numbers() {
    int question_numbers[NUM_QUESTIONS];
    int num_used = 0; //Keeps track of the number of question numbers we've added to the array.
    int min = 1; 
    int max = 5; //The number of questions we have
    int range = max - min + 1;

    srand(time(NULL));

    while (num_used < NUM_QUESTIONS) {
        // Generate a random number within the range
        int questionNumber = (rand() % range) + min;

        // Check if the number is already used
        int i;
        for (i = 0; i < num_used; i++) {
            if (questionNumber == question_numbers[i]) {
                break;
            }
        }

        // If the number is not used, add it to the list
        if (i == num_used) {
            question_numbers[num_used] = questionNumber;
            num_used++;
        }
    }
    //This is the random numbers generated that will be used as the question IDs we access.
    printf("NUMBERS: %d | %d | %d | %d | %d\n", question_numbers[0], question_numbers[1], question_numbers[2], question_numbers[3], question_numbers[4]);

    // Dynamically allocate an array and copy the contents of the question_numbers array to it
    int* random_numbers = malloc(sizeof(int) * NUM_QUESTIONS);
    memcpy(random_numbers, question_numbers, sizeof(int) * NUM_QUESTIONS);

    return random_numbers;
}

//Used to check if question number is in the array holding the question numbers we want. Accessed in read_questions_file().
int inArray(int val, int arr[], int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == val) {
            return 1; // found
        }
    }
    return 0; // not found
}

Question* read_questions_file(){
    char *filename = "questions.txt";

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { 
        perror("Error opening file");
        return NULL;
    }
    int *question_numbers = generate_questions_numbers();
    Question *questions = malloc(NUM_QUESTIONS * sizeof(Question));
    
    if (questions == NULL) {
        perror("Error allocating memory");
        fclose(fp);
        return NULL;
    }

    // Read each question from the file and store it in the array
    int i = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp) && i < NUM_QUESTIONS) {
        int id;
        char question[256];
        char option_a[OPTION_SIZE];
        char option_b[OPTION_SIZE];
        char option_c[OPTION_SIZE];
        char option_d[OPTION_SIZE];

        // Parse the line into question ID, question text, and answer
        if (sscanf(line, "%d,%255[^,],%24[^,],%24[^,],%24[^,],%24[^,\n]", &id, question, option_a, option_b, option_c, option_d) != 6) {
            printf("Failed to parse line %d in file %s\n", i+1, filename);
            continue;
        }

        // Check if the current question ID is in the list of question_numbers we want.
        if (inArray(id, question_numbers, NUM_QUESTIONS)) {
            questions[i].id = id;
            strcpy(questions[i].question, question);
            strcpy(questions[i].option_a, option_a);
            strcpy(questions[i].option_b, option_b);
            strcpy(questions[i].option_c, option_c);
            strcpy(questions[i].option_d, option_d);
            i++;
        }
    }

    // Close the file and free the memory used for the random numbers
    fclose(fp);
    free(question_numbers);

    return questions;
}


void send_questions(Question* questions, int sockfd){
    printf("Sending Questions...\n");
    // Determine the total size needed for the buffer
    int buffer_size = 0;
    for (int i = 0; i < NUM_QUESTIONS; i++) {
        if (questions[i].question[0] != '\0') {
            buffer_size += strlen(questions[i].question) + 3; // Add 3 for quotes and comma
        }
    }
    buffer_size += 2; // Add 2 for square brackets
    
    // Allocate memory for the buffer
    char* buffer = malloc(buffer_size);
    if (buffer == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    // Construct a JSON style output to send to the TM
    sprintf(buffer, "{");
    for (int i = 0; i < NUM_QUESTIONS; i++) {
        if (questions[i].question[0] != '\0') {
            sprintf(buffer + strlen(buffer), "\"%d\": {", questions[i].id);
            sprintf(buffer + strlen(buffer), "\"question\": \"%s\",", questions[i].question);
            sprintf(buffer + strlen(buffer), "\"option_a\": \"%s\",", questions[i].option_a);
            sprintf(buffer + strlen(buffer), "\"option_b\": \"%s\",", questions[i].option_b);
            sprintf(buffer + strlen(buffer), "\"option_c\": \"%s\",", questions[i].option_c);
            sprintf(buffer + strlen(buffer), "\"option_d\": \"%s\"", questions[i].option_d);
            sprintf(buffer + strlen(buffer), "},");
        }
    }
    sprintf(buffer + strlen(buffer) - 1, "}");
    

    // Send the Python code to the server
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }else {
    {
    //printf("Questions sent to TM\n");
    printf("Questions sent to TM\n%s\n",buffer); //Prints the list of questions sent to the TM.

    ///// SHOULD WE NOW IMPLEMENT ACKS AND NACKS HERE TO ENSURE IT WAS DELIVERED AND SEE IF WE NEED TO RESEND? //////
    }
    }
    
    // Free the buffer memory
    free(buffer);
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