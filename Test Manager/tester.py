import random
import json
import socket
import struct
import studentRecords as records

# each test can be assigned a session id 
# would allow concurrently running sessions to be distinguished
session_ids = []

QB_PORT = 9001

class Test:
    def __init__(self, student_id, question_bank_ip):
        # checks the flag for if the student has an active test going on
        existing_test = records.getActiveState(student_id)

        self.QB_IP = question_bank_ip
        self.student_id = student_id
        self.questions = []  # store each question as a json file
        self.question_counter = -1

        # create a session id to keep track of the test
        if len(session_ids) == 0:
            self.session_id = 0
        else:
            self.session_id = session_ids[-1] + 1

        if not existing_test:  # if the user has not previously started a test
            self.questions = self.get_question_dict()
            records.setGrade(student_id, 0)

            # puts all the questions into a json folder
            test_data = {}
            for index, question in enumerate(self.questions):
                test_data[index] = question
            records.setTestData(self.student_id, test_data)

            records.setTestActiveState(student_id, active=True)

        else:  # if state is to be resumed i.e. the user is halfway through a test
            test_data = records.getTestData(student_id)
            for key in test_data.keys():
                self.questions.append(test_data[key])


    # For refreshing the current question
    def getCurrentQuestion(self):
        return json.dumps(self.questions[self.question_counter])
    
    # For when the next question button is pressed
    def nextQuestion(self):
        # if self.question_counter >= self.getNumQuestions() - 1:
        #     return json.dumps({"message": "can't go forward anymore"})
        if self.question_counter < self.getNumQuestions() - 1:
            self.question_counter += 1

        # turns the dict into a json file (str)
        return json.dumps(self.questions[self.question_counter])

    # For when the previous question button is pressed
    def previousQuestion(self):
        # if self.question_counter == 0:
        #     return json.dumps({"message": "can't go back anymore"})
        if self.question_counter != 0:
            self.question_counter -= 1

        # turns the dict into a json file (str)
        return json.dumps(self.questions[self.question_counter])

    # returns the number of questions in the test session
    def getNumQuestions(self):
        return len(self.questions)

    def getCurrentQuestionNum(self):
        return self.question_counter

    # save the state of the current question to a file
    def saveState(self):
        pass

    def generate_question_dict(self, question_number):
        remaining_attempts = 3
        question_id = random.randint(0, 1000)
        temp_dict = {
            "question_id": str(question_id),
            "question_number": str(question_number),
            "question": "WHat is this ____" + str(random.randint(0, 100)),
            "remaining_attempts": str(remaining_attempts),
            "options": {
                "option_a": "test a" + str(random.randint(0, 100)),
                "option_b": "test b" + str(random.randint(0, 100)),
                "option_c": "test c" + str(random.randint(0, 100)),
                "option_d": "test d" + str(random.randint(0, 100))
            },
            "message": ""
        }

        return temp_dict

    def get_question_dict(self):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)

        header = "questions"  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)

        message = "2"
        data = header_len_bytes + header.encode() + message.encode()

        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        response = sock.recv(2048)  # Awaits a response.
        message = str(response, 'utf-8')
        dict = json.loads(message)
        temp_questions_list = []
        i=1
        for key in dict.keys():
            remaining_attempts = 3
            question_id = random.randint(0, 1000)
            temp_dict = {
                "question_id": str(key),
                "question_number": str(i),
                "question": dict[key]["question"],
                "remaining_attempts": str(remaining_attempts),
                "options": {
                    "option_a": dict[key]["option_a"],
                    "option_b": dict[key]["option_b"],
                    "option_c": dict[key]["option_c"],
                    "option_d": dict[key]["option_d"]
                },
                "message": ""
            }
            i+=1
            temp_questions_list.append(temp_dict)
        return temp_questions_list

    def format_question(self, question_number, question_id, ):
        remaining_attempts = 3
        question_id = random.randint(0, 1000)
        temp_dict = {
            "question_id": str(question_id),
            "question_number": str(question_number),
            "question": "WHat is this ____" + str(random.randint(0, 100)),
            "remaining_attempts": str(remaining_attempts),
            "options": {
                "option_a": "test a" + str(random.randint(0, 100)),
                "option_b": "test b" + str(random.randint(0, 100)),
                "option_c": "test c" + str(random.randint(0, 100)),
                "option_d": "test d" + str(random.randint(0, 100))
            },
            "message": ""
        }

        return temp_dict


    def getAnswer(self, question_bank, question_number, answer):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)

        header = "mc_answer"  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)
        questionID = getQuestionID(question_bank.questions, question_number+1)
        message = "{}={}".format(questionID, answer) #The 'question_id' being sent here is just the question number, not the ID
        data = header_len_bytes + header.encode() + message.encode()

        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        response = sock.recv(1024)  # Awaits a response.
        msg = str(response, 'utf-8')
        if (msg == 'T'):
            print(f"Received response: '{msg}', answer was CORRECT")
            return True
        else:
            print(f"Received response: '{msg}', answer was INCORRECT")
            return False
    
    def getCorrectAnswer(self, question_bank, question_number):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)

        header = "sendAnswer"  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        questionID = getQuestionID(question_bank.questions, question_number+1)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)
        data = header_len_bytes + header.encode() + str(questionID).encode()

        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        response = sock.recv(1024)  # Awaits a response.
        answer = str(response, 'utf-8')
        return answer


def getQuestionID(questions_list, current_question_number):
        for question in questions_list:
            question_id = question["question_id"]
            question_number = question['question_number']
            if int(question_number) == int(current_question_number):
                return question_id
                


def connect_to_server(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (host, port)
    try:
        sock.connect(server_address)
        print("Connection successful!")
    except socket.error as e:
        print(f"Error connecting to server: {e}")
        exit(1)
    return sock
