#ifndef QB_H
#define QB_H

#define BUF_SIZE 1024
#define MAX_PAYLOAD_LEN 2048    
#define MAX_LINE_LENGTH 2048
#define NUM_QUESTIONS 5
#define OPTION_SIZE 255
#define NUM_OF_AVAILABLE_QUESTIONS 25 //This is the total number of questions in the question set to pick from.

struct message {
    int length; //Length of header
    char payload[MAX_PAYLOAD_LEN];
};

typedef struct {
    int id;
    char question[BUF_SIZE]; // either needs to be unsigned, malloc or change the size
    char option_a[OPTION_SIZE];
    char option_b[OPTION_SIZE];
    char option_c[OPTION_SIZE];
    char option_d[OPTION_SIZE];
} Question;

typedef struct {
    int id;
    char answer[64];
}Answer;


void create_socket(int *sockfd);
void bind_socket(int sockfd);
void listen_for_connections(int sockfd);
void handle_connection(int sockfd);
void close_connection(int connfd);
void get_local_ip(char *ip, int size);
Question* read_questions_file(int num_questions, char *filename);
int* generate_questions_numbers(int num_questions);
void send_questions(Question* questions, int sockfd, int numOfQuestions);
int mark_MC_Question(int question_id, char *student_answer, char *filename);
char *retreiveAnswer(char *question_id);
void removeNewline(char* str);
Question* read_p_questions_file(int num_questions, char *filename);
void send_p_questions(Question* questions, int sockfd, int numOfQuestions, char *language);
#endif
