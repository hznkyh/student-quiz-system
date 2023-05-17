#ifndef QB_H
#define QB_H

#define BUF_SIZE 1024
#define MAX_PAYLOAD_LEN 1024
#define MAX_LINE_LENGTH 1024
#define NUM_QUESTIONS 5
#define OPTION_SIZE 24
#define NUM_OF_AVAILABLE_QUESTIONS 27 //This is the total number of questions in the question set to pick from.

struct message {
    int length; //Length of header
    char payload[MAX_PAYLOAD_LEN];
};

typedef struct {
    int id;
    char question[256];
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
Question* read_questions_file(void);
int* generate_questions_numbers(void);
void send_questions(Question* questions, int sockfd);
int mark_MC_Question(int question_id, char *student_answer);
char *retreiveAnswer(char *question_id);
void removeNewline(char* str);
#endif
