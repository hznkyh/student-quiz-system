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
#include <stdbool.h>

//gcc QB.c -o QB 



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

// Create Socket
void create_socket(int *sockfd) {
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
}

// Bind Socket
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
    
    memset(msg.payload, 0, sizeof(msg.payload));
    //Receive the Message header
    n = recv(connfd, &msg, sizeof(msg), 0);
        if (n <= 0) {
            perror("recv error");
            exit(1);
        }
    msg.length = ntohl(msg.length);
    //msg.payload[strlen(msg.payload)-3] = '\0'; //Shorter messages seem to have issues of adding random characters. 
                                                //Couldn't figure out why, minimum msg length has to be 12 character to be accurate
    char *source = msg.payload;
    char header[msg.length];
    memset(header, 0, sizeof(header));
    for (int i = 0; i < msg.length; i++) {
        strncat(header, &source[i], 1);
    }
    //memmove(header, header+1, strlen(header)); //removes the non-printable STX control character.
    removeNewline(header); 
    char *newPayload = msg.payload+msg.length;
    
    printf("MESSAGE RECEIVED: '%s'",msg.payload);
    printf("HEADER: '%s' | PAYLOAD: '%s'\n",header, newPayload);

    Question* questions;
    
    if (strcmp(header, "mc_questions") == 0){
        int numOfQuestions = atol(newPayload); //Payload is how many questions TM wants.
        printf("TM requested %d mc questions...\n",numOfQuestions);\
        questions = read_questions_file(numOfQuestions, "mc_questions.txt");
        send_questions(questions, connfd, numOfQuestions);
    
    }
    else if (strcmp(header, "c_questions") == 0){
        int numOfQuestions = atol(newPayload); //Payload is how many questions TM wants.
        printf("TM requested %d C programming questions...\n",numOfQuestions);
        questions = read_p_questions_file(numOfQuestions, "c_questions.txt");
        send_p_questions(questions, connfd, numOfQuestions, "c");
    
    }
    else if (strcmp(header, "py_questions") == 0){
        int numOfQuestions = atol(newPayload); //Payload is how many questions TM wants.
        printf("TM requested %d C programming questions...\n",numOfQuestions);
        questions = read_p_questions_file(numOfQuestions, "py_questions.txt");
        send_p_questions(questions, connfd, numOfQuestions, "py");
    
    }
    else if(strcmp(header, "mark_mc_answer") == 0){
        char *qID;
        char *qAnswer;
        qID = strtok(newPayload, "=");
        qAnswer = strtok(NULL, "=");
        int correct = mark_MC_Question(atol(qID), qAnswer, "mc_answers.txt");
        char c_value = correct ? 'T' : 'F'; //Have to do this because sending a 0 or 1 over a socket doesn't work.
       if (send(connfd, &c_value, sizeof(c_value), 0) < 0) {
            perror("Result send failed");
            exit(EXIT_FAILURE);
        }else {
            printf("Result sent to QB ('%c')\n",c_value);
        }
    }else if(strcmp(header, "mark_c_answer") == 0){ //Mark C programming question
        printf("Will mark the C question now...\n");

    }else if(strcmp(header, "mark_py_answer") == 0){ //Mark Python programming question
        printf("Will mark the Python question now...\n");

    }else if (strcmp(header, "send_c_answer") == 0){
        printf("Will send the C answer now...\n");
    
    }else if (strcmp(header, "send_py_answer") == 0){
        printf("Will send the Python answer now...\n");
    
    }else if (strcmp(header, "send_mc_answer") == 0){
        char *answer = retreiveAnswer(newPayload);
        if (send(connfd, answer, strlen(answer), 0) < 0) {
            perror("Result send failed");
            exit(EXIT_FAILURE);
        }
        printf("Answer sent to TM: '%s'",answer);
    }else{
        printf("ERROR: Header '%s' not recognised.\n",header);
    }

}

void removeNewline(char* str) {
    size_t newlinePos = strcspn(str, "\n");  // Find the position of the newline character

    if (str[newlinePos] == '\n') {  // If a newline character is found
        str[newlinePos] = '\0';     // Replace it with a null terminator
    }
}

char* retreiveAnswer(char *qID){
    printf("Getting Answer...\n");
    int wanted_id = atol(qID);
    char *answer;
    char *filename = "answers.txt";

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { 
        perror("Error opening file");
        return 0;
    }
    int current_id;
    char* current_answer = malloc(128 * sizeof(char));
    if (current_answer == NULL) {
        return 0;
    }

    int i = 0;
    char line[MAX_LINE_LENGTH];
    bool found_id = false;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d,%128[^,\n]", &current_id, current_answer) != 2) {
            printf("Failed to parse line %d in file %s\n", i+1, filename);
            continue;
        }

        if (current_id == wanted_id){
            printf("%s\n",line);
            printf("Answer found: '%s'\n",current_answer);
            answer = current_answer;
            found_id = true;
            break; // Exit the loop since the desired ID has been found
        }
    }

    if (!found_id) {
        printf("Desired ID not found in file %s\n", filename);
        return 0;
    }else{
        return answer;
    }

}

//Used to randomly generate a number within the range of the number of questions we have. Ensures no duplicates.
int* generate_questions_numbers(int num_questions) {
    int question_numbers[num_questions];
    int num_used = 0; //Keeps track of the number of question numbers we've added to the array.
    int min = 1; 
    int max = 27; //The number of questions we have
    int range = max - min + 1;

    srand(time(NULL));

    while (num_used <num_questions) {
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
    printf("Q IDs to send: ");
    for(int i=0; i < num_questions; i++){
        printf("%d | ", question_numbers[i]);
    }
    printf("\n");

    // Dynamically allocate an array and copy the contents of the question_numbers array to it
    int* random_numbers = malloc(sizeof(int) *num_questions);
    memcpy(random_numbers, question_numbers, sizeof(int) * num_questions);

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

Question* read_questions_file(int num_questions, char *filename){

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { 
        perror("Error opening file");
        return NULL;
    }
    int *question_numbers = generate_questions_numbers(num_questions);
    Question *questions = malloc(num_questions * sizeof(Question));
    
    if (questions == NULL) {
        perror("Error allocating memory");
        fclose(fp);
        return NULL;
    }

    // Read each question from the file and store it in the array
    int i = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp) && i < num_questions) {
        int id;
        char question[256];
        char option_a[OPTION_SIZE];
        char option_b[OPTION_SIZE];
        char option_c[OPTION_SIZE];
        char option_d[OPTION_SIZE];

        // Parse the line into question ID, question text, and answer
        if (sscanf(line, "%d,%1024[^,],%255[^,],%255[^,],%255[^,],%255[^,\n]", &id, question, option_a, option_b, option_c, option_d) != 6) {
            printf("Failed to parse line %d in file %s\n", i+1, filename);
            //printf("LINE:line)
            continue;
        }

        // Check if the current question ID is in the list of question_numbers we want.
        if (inArray(id, question_numbers, NUM_OF_AVAILABLE_QUESTIONS)) {
            questions[i].id = id;
            strcpy(questions[i].question, question);
            strcpy(questions[i].option_a, option_a);
            strcpy(questions[i].option_b, option_b);
            strcpy(questions[i].option_c, option_c);
            strcpy(questions[i].option_d, option_d);
            printf("Q'%d':'%s'\n",questions[i].id,questions[i].question);
            printf("a:'%s' \nb:'%s'\nc:'%s'\nd:'%s'\n",questions[i].option_a,questions[i].option_b,questions[i].option_c,questions[i].option_d);
            i++;
        }
    }

    // Close the file and free the memory used for the random numbers
    fclose(fp);
    free(question_numbers);

    printf("Questions generated.\n");
    return questions;
}

Question* read_p_questions_file(int num_questions, char *filename){
    printf("opening file '%s'\n",filename);
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { 
        perror("Error opening file");
        return NULL;
    }
    int *question_numbers = generate_questions_numbers(num_questions);
    Question *questions = malloc(num_questions * sizeof(Question));
    
    if (questions == NULL) {
        perror("Error allocating memory");
        fclose(fp);
        return NULL;
    }

    // Read each question from the file and store it in the array
    int i = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp) && i < num_questions) {
        int id;
        char question[256];

        // Parse the line into question ID, question text, and answer
        if (sscanf(line, "%d,%255[^,\n]", &id, question) != 2) {
            printf("Failed to parse line %d in file %s\n", i+1, filename);
            continue;
        }

        // Check if the current question ID is in the list of question_numbers we want.
        if (inArray(id, question_numbers, NUM_OF_AVAILABLE_QUESTIONS)) {
            questions[i].id = id;
            strcpy(questions[i].question, question);
            printf("Q'%d':'%s'\n",questions[i].id,questions[i].question);
            i++;
        }
    }

    // Close the file and free the memory used for the random numbers
    fclose(fp);
    free(question_numbers);

    printf("Questions generated.\n");
    return questions;
}

int mark_MC_Question(int question_id, char *student_answer, char *filename) {
    printf("Marking MC Answer\n");

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { 
        perror("Error opening file");
        return 0;
    }
    
    int current_id;
    char line[MAX_LINE_LENGTH];
    char* current_answer = NULL; // Initialize to NULL
    
    int i = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d,%128[^,\n]", &current_id, line) != 2) {
            printf("Failed to parse line %d in file %s\n", i+1, filename);
            continue;
        }

        if (current_id == question_id) {
            current_answer = strdup(line); // Allocate memory and copy the line
            break;
        }
    }

    if (current_answer != NULL) {
        if (strcmp(current_answer, student_answer) == 0) {
            free(current_answer);
            return 1;
        }
        
        printf("Comparison: '%s' == '%s' ?", current_answer, student_answer);
    } else {
        printf("Question ID %d not found in file %s\n", question_id, filename);
    }

    return 0;
}

//The file name provided will either be the mc question set or programming questions.
void send_questions(Question* questions, int sockfd, int numOfQuestions){
    printf("Sending Questions...\n");

    int buffer_size = 2060;
    for (int i = 0; i < numOfQuestions; i++) {
        if (questions[i].question[0] != '\0') {
            buffer_size += strlen(questions[i].question);
        }
    }
    
    char* buffer = malloc(buffer_size);
    if (buffer == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    // Construct a JSON style output to send to the TM
    sprintf(buffer, "{");
    for (int i = 0; i < numOfQuestions; i++) {
        if (questions[i].question[0] != '\0') {
            sprintf(buffer + strlen(buffer), "\"%d\": {", questions[i].id);
            sprintf(buffer + strlen(buffer), "\"question\": \"%s\",", questions[i].question);
            sprintf(buffer + strlen(buffer), "\"type\": \"%s\",", "mc");
            sprintf(buffer + strlen(buffer), "\"option_a\": \"%s\",", questions[i].option_a);
            sprintf(buffer + strlen(buffer), "\"option_b\": \"%s\",", questions[i].option_b);
            sprintf(buffer + strlen(buffer), "\"option_c\": \"%s\",", questions[i].option_c);
            sprintf(buffer + strlen(buffer), "\"option_d\": \"%s\"", questions[i].option_d);
            sprintf(buffer + strlen(buffer), "},");
        }
    }
    if (strlen(buffer) > 0) {
        sprintf(buffer + strlen(buffer) - 1, "}");
    }

    // Send the Python code to the server
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("send failed");
        //free(buffer);
        exit(EXIT_FAILURE);
    }
    
    printf("Questions sent to TM\n");
    //printf("Questions sent to TM\n%s\n",buffer); //Prints the list of questions sent to the TM.
    
    // Free the buffer memory
    //free(buffer);
    free(questions);
}

void send_p_questions(Question* questions, int sockfd, int numOfQuestions, char *language){
    printf("Sending Programming Questions...\n");

    int buffer_size = 2060;
    for (int i = 0; i < numOfQuestions; i++) {
        if (questions[i].question[0] != '\0') {
            buffer_size += strlen(questions[i].question);
        }
    }
    
    char* buffer = malloc(buffer_size);
    if (buffer == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    // Construct a JSON style output to send to the TM
    sprintf(buffer, "{");
    for (int i = 0; i < numOfQuestions; i++) {
        if (questions[i].question[0] != '\0') {
            sprintf(buffer + strlen(buffer), "\"%d\": {", questions[i].id);
            sprintf(buffer + strlen(buffer), "\"question\": \"%s\",", questions[i].question);
            sprintf(buffer + strlen(buffer), "\"type\": \"%s\"", language);
            sprintf(buffer + strlen(buffer), "},");
        }
    }
    if (strlen(buffer) > 0) {
        sprintf(buffer + strlen(buffer) - 1, "}");
    }
    // Send the Python code to the server
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("send failed");
        //free(buffer);
        exit(EXIT_FAILURE);
    }
    
    printf("Programming Questions sent to TM\n");
    printf("Question Set: %s\n",buffer);
    //printf("Questions sent to TM\n%s\n",buffer); //Prints the list of questions sent to the TM.
    
    // Free the buffer memory
    // free(buffer); //Causing random problems...

    free(questions);
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