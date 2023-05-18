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
#include <ctype.h>
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


// Function to trim leading and trailing whitespace characters
void trim(char* str) {
    int start = 0;
    int end = strlen(str) - 1;

    // Find the index of the first non-whitespace character
    while (isspace(str[start])) {
        start++;
    }

    // Find the index of the last non-whitespace character
    while (end >= 0 && isspace(str[end])) {
        end--;
    }

    // Shift the remaining characters to the beginning of the string
    int i, j;
    for (i = start, j = 0; i <= end; i++, j++) {
        str[j] = str[i];
    }

    // Add the null terminator at the end of the trimmed string
    str[j] = '\0';
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
    
    //CHECKS THE HEADER TO SEE WHAT IT IS BEING ASKED TO DO
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
    }
    else if(strcmp(header, "mark_c_answer") == 0){ //Mark C programming question
        char* question_id;
        char *user_code;
        char* delimiter = strchr(newPayload, '='); 
        // Split the input
        *delimiter = '\0'; // Null-terminate the string at the delimiter
        question_id = newPayload;
        user_code = delimiter + 1;
        printf("USER CODE:'%s'",user_code);
        if(atol(question_id) == 1){ //If answer was for question with ID 1
            printf("Will mark the C question one now...(%s)\n",question_id);
            //WE ADD A MAIN FUNCTION AND stdio.h TO THE USERS FILE SO WE CAN RUN IT AND GET THE EXPECTED OUTPUT.
            char* insertMain = "#include <stdio.h>\n#include <string.h>\nvoid reverseString(char* str);\n\nint main() {\n\tchar input[] = \"amazing\";\n\treverseString(input);\n\treturn 0;\n}\n\n";
            int finalSize = strlen(insertMain) + strlen(user_code) + 1;
            char* finalCode = (char*)malloc(finalSize);

            strcpy(finalCode, insertMain);
            strcat(finalCode, user_code);
            printf("QID:'%s'\n",question_id);
            printf("%s\n", finalCode);

            saveUserCode(finalCode);
            free(finalCode);
            compileUserCode();
            runUserCode();
            processOutputAndErrors();
        }
        else if(atol(question_id)==2){ //If answer was for question with ID 2
            printf("Will mark the C question two now... (%s)\n",question_id);

            //WE ADD A MAIN FUNCTION AND stdio.h TO THE USERS FILE SO WE CAN RUN IT AND GET THE EXPECTED OUTPUT.
            char* insertMain = "#include <stdio.h>\n\nvoid stringLength(char* str);\n\nint main() {\n\tchar input[] = \"Hello, World!\";\n\tstringLength(input);\n\treturn 0;\n}\n\n";
            int finalSize = strlen(insertMain) + strlen(user_code) + 1;
            char* finalCode = (char*)malloc(finalSize);

            strcpy(finalCode, insertMain);
            strcat(finalCode, user_code);
            printf("qid:'%s'\n",question_id);
            printf("%s\n", finalCode);

            saveUserCode(finalCode);
            free(finalCode);
            compileUserCode();
            int out = runUserCode();
            if (!out){
                if (send(connfd, "F", strlen("F"), 0) < 0) {
                    perror("Result send failed");
                    exit(EXIT_FAILURE);
                    }
            }
            char* result = processOutputAndErrors();
        }
        else{
            printf("Unknwon Question ID '%s",question_id);
        }
        

    }
    else if(strcmp(header, "mark_py_answer") == 0){ //Mark Python programming question
        printf("Will mark the Python question now...\n");
        char *qID;
        char *user_code;
        qID = strtok(newPayload, "=");
        user_code = strtok(NULL, "=");
        
        printf("%s",qID);
        printf("%s\n",user_code);

        if (atol(qID) == 1) {
            char* insertMain = "\nif __name__ == '__main__':\n    result = reverse(\"spaces\")\n    print(result)\n";
            int finalSize = strlen(insertMain) + strlen(user_code) + 1;
            char* finalCode = (char*)malloc(finalSize + 1);
            strcpy(finalCode, user_code);
            strcat(finalCode, insertMain);

            savePythonUserCode(finalCode);
            runUserCodePy();
            char* outputContent = processOutputAndErrorPy();
            const char* anotherString = "secaps";
            printf("anotherString: %s\n", anotherString);

            if (outputContent != NULL) {
                printf("Output Content:\n%s\n", outputContent);

                // Trim leading and trailing whitespace characters from outputContent
                trim(outputContent);

                printf("Trimmed Output Content:\n%s\n", outputContent);

                if (strcmp(outputContent, anotherString) == 0) {
                    printf("Test case passed.\n");
                } else {
                    printf("Failed test case.\n");
                }

                free(outputContent);
            } else {
                printf("Failed to retrieve output content.\n");
            }
        }else if(atol(qID) ==2){
            char *insertMain = "\nif __name__ == '__main__':\n    result = string_length(\"spaces\")\n    print(result)\n";
            int finalSize = strlen(insertMain) + strlen(user_code) + 1;
            char* finalCode = (char*)malloc(finalSize + 1);
            strcpy(finalCode, user_code);
            strcat(finalCode, insertMain);

            
            savePythonUserCode(finalCode);
            runUserCodePy();
            char* outputContent = processOutputAndErrorPy();
            const char* anotherString = "6";
            printf("anotherString: %s\n", anotherString);
            if (outputContent != NULL) {
                printf("Output Content:\n%s\n", outputContent);

                // Trim leading and trailing whitespace characters from outputContent
                trim(outputContent);

                printf("Trimmed Output Content:\n%s\n", outputContent);

                if (strcmp(outputContent, anotherString) == 0) {
                    printf("Test case passed.\n");
                    if (send(connfd, "True", strlen("True"), 0) < 0) {
                        perror("Result send failed");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    printf("Failed test case.\n");
                    if (send(connfd, "False", strlen("False"), 0) < 0) {
                        perror("Result send failed");
                        exit(EXIT_FAILURE);
                    }
                }

                free(outputContent);
            } else {
                printf("Failed to retrieve output content.\n");
            }

        }
  

    }
    else if (strcmp(header, "send_c_answer") == 0){
        printf("Will send the C answer now...\n");
        if (send(connfd, "F", strlen("F"), 0) < 0) {
            perror("Result send failed");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(header, "send_py_answer") == 0){
        printf("Will send the Python answer now...\n");
    
    }
    else if (strcmp(header, "send_mc_answer") == 0){
        char *answer = retrieveAnswer(newPayload);
        if (send(connfd, answer, strlen(answer), 0) < 0) {
            perror("Result send failed");
            exit(EXIT_FAILURE);
        }
        printf("Answer sent to TM: '%s'",answer);
    }
    else{
        printf("ERROR: Header '%s' not recognised.\n",header);
    }

}


// Save the Python answer from the user.
void savePythonUserCode(char* code){
    FILE* file = fopen("usercode_py.py", "w");
        if (file == NULL) {
            printf("Failed to open the file for writing.\n");
            return;
        }
        fputs(code, file);
        fclose(file);
}

// Run python code
void runUserCodePy() {
    // Assume we only allow python code
    int result = system("python3 usercode_py.py > output_py.txt 2> errors_py.txt");
    if (result != 0) {
        printf("Execution error.\n");
    } else {
        printf("Program executed successfully.\n");
    }
}

// Save C questions to a file ready to compile.
void saveUserCode(char* code) {
        FILE* file = fopen("usercode.c", "w");
        if (file == NULL) {
            printf("Failed to open the file for writing.\n");
            return;
        }
        fputs(code, file);
        fclose(file);
}

// process the output and error for py files.
char* processOutputAndErrorPy() {
    FILE* outputFile = fopen("output_py.txt", "r");
    if (outputFile != NULL) {
        printf("Program output:\n");
        char buffer[256];
        char* outputContent = malloc(sizeof(char));  // Allocate initial memory
        size_t outputSize = 0;  // Track the size of the output content

        while (fgets(buffer, sizeof(buffer), outputFile) != NULL) {
            printf("%s", buffer);

            // Reallocate memory for the output content
            outputContent = realloc(outputContent, (outputSize + strlen(buffer) + 1) * sizeof(char));
            strcat(outputContent, buffer);
            outputSize += strlen(buffer);
        }
        
        fclose(outputFile);

        // Add a null terminator to the output content
        outputContent[outputSize] = '\0';

        return outputContent;
    } else {
        printf("Failed to open the output file.\n");
        return NULL;  // Return NULL if the file couldn't be opened
    }
}
// Compile C code 
void compileUserCode() {
    //Remove any previous file so it doesn't interfere with this new execution if this compile fails.
    if (remove("usercode") != 0) {
        perror("Failed to delete previous usercode executable");
        return;
    }
    int result = system("gcc usercode.c -o usercode");
    if (result != 0) {
        printf("Compilation error.\n");
    } else {
        printf("Code compiled successfully.\n");
    }
}

// Run code C 
int runUserCode() {
    //Sometimes contents of the file stays and returns result of the previous attempt should the execute fail
    FILE* file = fopen("output.txt", "w+");
    if (file == NULL) {
        perror("Failed to open output.txt");
        return 1;
    }
    fclose(file);
    

    // Execute the command
    int result = system("./usercode > output.txt 2> errors.txt");
    if (result != 0) {
        printf("Execution error.\n");
        return 1;
    } else {
        printf("Program executed successfully.\n");
        return 0;
    }
    
}

// process the output and error for c files.
char* processOutputAndErrors() {
    char outputBuffer[256] = "";
    FILE* outputFile = fopen("output.txt", "r");
    if (outputFile != NULL) {
        printf("Program output:\n");
        while (fgets(outputBuffer, 256, outputFile) != NULL) {
            printf("%s", outputBuffer);
        }
        fclose(outputFile);
    } else {
        printf("Failed to open the output file.\n");
        return strdup("Error: Failed to open the output file.");
    }

    char errorsBuffer[256] = "";
    FILE* errorsFile = fopen("errors.txt", "r");
    if (errorsFile != NULL) {
        printf("Error messages:\n");
        while (fgets(errorsBuffer, 256, errorsFile) != NULL) {
            printf("%s", errorsBuffer);
        }
        fclose(errorsFile);
    } else {
        printf("Failed to open the errors file.\n");
        return strdup("Error: Failed to open the errors file.");
    }

    return strdup(outputBuffer);
}

//This was used to remove some random new line characters in the header that sometime's popped up.
void removeNewline(char* str) {
    size_t newlinePos = strcspn(str, "\n");  // Find the position of the newline character

    if (str[newlinePos] == '\n') {  // If a newline character is found
        str[newlinePos] = '\0';     // Replace it with a null terminator
    }
}

//This is used to retrieve the answer when people get the question wrong 3 times.
char* retrieveAnswer(char *qID){
    printf("Getting Answer...\n");
    int wanted_id = atol(qID);
    char *answer;
    char *filename = "mc_answers.txt";

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


//gets the coding solution for the given problem. Need to know the language so we can open the correct file.
// language arg is either "c" or "py"
char* retrieveCodingSolution(char *qID, char *language){ 
    printf("Getting Solution...\n");
    int wanted_id = atol(qID);
    char *answer;
    char *filename = "py_solutions.txt";
    if(strcmp(language, "c")){
        char *filename = "c_solutions.txt";
    }
    

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



// Used to randomly generate a number within the range of the number of questions we have. Ensures no duplicates.
// It takes in the min and max values of what question number range there is corresponding to Question IDs in the files.
int* generate_questions_numbers(int num_questions, int min, int max) {
    int question_numbers[num_questions];
    int num_used = 0; //Keeps track of the number of question numbers we've added to the array.
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

/*
* This is used for multi choice question files to extract the data needed to display onto the page.
*/
Question* read_questions_file(int num_questions, char *filename){

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { 
        perror("Error opening file");
        return NULL;
    }
    int *question_numbers = generate_questions_numbers(num_questions, 3 , 26);
    Question *questions = malloc(num_questions * sizeof(Question));
    
    if (questions == NULL) {
        perror("Error allocating memory");
        fclose(fp);
        return NULL;
    }


    int i = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp) && i < num_questions) {
        int id;
        char question[256];
        char option_a[OPTION_SIZE];
        char option_b[OPTION_SIZE];
        char option_c[OPTION_SIZE];
        char option_d[OPTION_SIZE];

        // Goes through the file and grabs the question id, question and all 4 options for that particular quesstion.
        if (sscanf(line, "%d,%255[^,],%255[^,],%255[^,],%255[^,],%255[^,\n]", &id, question, option_a, option_b, option_c, option_d) != 6) {
            printf("Failed to parse line %d in file %s\n", i+1, filename);
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
            // printf("Q'%d':'%s'\n",questions[i].id,questions[i].question);
            // printf("a:'%s' \nb:'%s'\nc:'%s'\nd:'%s'\n",questions[i].option_a,questions[i].option_b,questions[i].option_c,questions[i].option_d);
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
    int *question_numbers = generate_questions_numbers(num_questions, 1, 2);
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