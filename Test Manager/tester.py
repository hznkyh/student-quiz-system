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
    def get_current_question(self):
        return json.dumps(self.questions[self.question_counter])
    
    # For when the next question button is pressed
    def next_question(self):
        if self.question_counter < self.get_num_questions() - 1:
            self.question_counter += 1

        # turns the dict into a json file (str)
        return json.dumps(self.questions[self.question_counter])

    # For when the previous question button is pressed
    def previous_question(self):
        if self.question_counter != 0:
            self.question_counter -= 1

        # turns the dict into a json file (str)
        return json.dumps(self.questions[self.question_counter])

    # returns the number of questions in the test session
    def get_num_questions(self):
        return len(self.questions)

    def get_current_questionNum(self):
        return self.question_counter

    # save the state of the current question to a file
    def save_state(self):
        pass


    def get_question_dict(self):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)
        header = "mc_questions"  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)
        message = "3"
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
                "type": dict[key]["type"],
                "options": {
                    "option_a": dict[key]["option_a"],
                    "option_b": dict[key]["option_b"],
                    "option_c": dict[key]["option_c"],
                    "option_d": dict[key]["option_d"]
                },
                "message": ""
            }
            i += 1
            temp_questions_list.append(temp_dict)
        
        print(temp_questions_list)
        sock = connect_to_server(self.QB_IP, QB_PORT)
        # Randomly choose between requesting C questions and Python Questions
        # selected_question_set = random.choice(["c_questions", "py_questions"])
        header = "c_questions"#selected_question_set  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)
        message = "2"
        data = header_len_bytes + header.encode() + message.encode()
        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        response = sock.recv(2048)
        message = str(response, 'utf-8')
        dict = json.loads(message)
        for key in dict.keys():
            remaining_attempts = 3
            question_id = random.randint(0, 1000)
            temp_dict = {
                "question_id": str(key), 
                "question_number": str(i),
                "question": dict[key]["question"],
                "remaining_attempts": str(remaining_attempts),
                "type": dict[key]["type"]
            }
            i += 1
            temp_questions_list.append(temp_dict)
        return temp_questions_list

    def get_answer(self, question_bank, question_number, answer):
        # Check question type and perform relevant action
        question_type = question_bank.questions[question_number]["type"]
        print(f"Q-Type {question_type}")
        if question_type == "mc":
            return self.mark_multiple_choice_answer(question_bank, question_number, answer)
        elif question_type == "py" or question_type == "c":
            return self.mark_prog_answer(question_bank, question_number, answer)
        else:
            return None  # Handle other question types as needed


    def get_correct_answer(self, question_bank, question_number, answer):
        # Check question type and perform relevant action
        question_type = question_bank.questions[question_number]["type"]
        if question_type == "mc":
            return self.get_multiple_choice_answer(question_bank, question_number)
        elif question_type == "py" or question_type == "c":
            return self.get_prog_answer(question_bank, question_number, question_type)
        else:
            return None  # Handle other question types as needed
    
    def mark_multiple_choice_answer(self, question_bank, question_number, answer):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)
        header = "mark_mc_answer"  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)
        questionID = get_question_id(question_bank.questions, question_number+1)
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
    
    def mark_prog_answer(self, question_bank, question_number, answer):
        question_type = question_bank.questions[question_number]["type"]
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)
        header = "mark_" + question_type + "_answer"
        header_len = len(header)
        questionID = get_question_id(question_bank.questions, question_number+1)
        message = "{}={}".format(questionID, answer)
        header_len_bytes = struct.pack("!I", header_len)
        data = header_len_bytes + header.encode() + message.encode()
        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        #response = sock.recv(2048)  # Awaits a response. #NOT WAITING ATM BECAUSE NOT MARKING IS COMING
        #answer = str(response, 'utf-8')
        return "Nothing for now" #NOT WAITING ATM BECAUSE NOT MARKING IS COMING

    
    #Returns the correct answer for a question, used when out of attempts.
    def get_multiple_choice_answer(self, question_bank, question_number):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)
        header = "send_mc_answer"  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        questionID = get_question_id(question_bank.questions, question_number+1)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)
        data = header_len_bytes + header.encode() + str(questionID).encode()

        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        response = sock.recv(2048)  # Awaits a response.
        answer = str(response, 'utf-8')
        return answer
    
    #Returns the correct answer output for a programming question, used when out of attempts.
    def get_prog_answer(self, question_bank, question_number, question_type):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)
        header = "send_" + question_type + "_answer" #This will be either "c"+"Answer" or "py"+"Answer"
        print(f"Sending header: {header}")
        header_len = len(header)
        questionID = get_question_id(question_bank.questions, question_number+1)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)
        data = header_len_bytes + header.encode() + str(questionID).encode()
        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        response = sock.recv(2048)  # Awaits a response.
        answer = str(response, 'utf-8')
        return answer
    

def get_question_id(questions_list, current_question_number):
    for question in questions_list:
        question_id = question["question_id"]
        question_number = question["question_number"]
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