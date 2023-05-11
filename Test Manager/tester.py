import random
import json
import socket
import struct

# each test can be assigned a session id 
# would allow concurrently running sessions to be distinguished
session_ids = []

QB_PORT = 9001

class Test:
    def __init__(self, student_id, question_bank_ip, resume_state=False, num_questions=10):
        if not resume_state:
            self.QB_IP = question_bank_ip
            self.student_id = student_id
            self.questions = []  # store each question as a json file
            self.question_counter = 0

            # create a session id to keep track of the test
            if len(session_ids) == 0:
                self.session_id = 0
            else:
                self.session_id = session_ids[-1] + 1

            for i in range(num_questions):
                # retrieve all the questions before the user begins the test
                # questions could be sent back to QB to be marked individually though
                self.questions.append(self.generate_question_html(i))

        else:  # if state is to be resumed
            # read file
            pass

    # For when the next question button is pressed
    def nextQuestion(self):
        if self.question_counter > self.getNumQuestions() - 1:
            return False

        self.question_counter += 1
        return self.questions[self.question_counter]

    """
    def generate_question_html(self, question_number, username):
        # TODO: grab question from based on username
        temp_dict = {
            "question_number": question_number,
            "question": "WHat is this ____" + str(random.randint(0, 100)),
            "options": {
                "option_a": "test a" + str(random.randint(0, 100)),
                "option_b": "test b" + str(random.randint(0, 100)),
                "option_c": "test c" + str(random.randint(0, 100)),
                "option_d": "test d" + str(random.randint(0, 100))
            }
        }

        temp_json = json.dumps(temp_dict)

        return temp_json
    """

    # For when the previous question button is pressed
    def previousQuestion(self):
        if self.question_counter == 0:
            return False

        self.question_counter -= 1
        return self.questions[self.question_counter]

    # returns the number of questions in the test session
    def getNumQuestions(self):
        return len(self.questions)

    # save the state of the current question to a file
    def saveState(self):
        pass


    def generate_question_html(self, question_number):
        temp_dict = {
            "question_number": question_number,
            "question": "WHat is this ____" + str(random.randint(0, 100)),
            "options": {
                "option_a": "test a" + str(random.randint(0, 100)),
                "option_b": "test b" + str(random.randint(0, 100)),
                "option_c": "test c" + str(random.randint(0, 100)),
                "option_d": "test d" + str(random.randint(0, 100))
            }
        }

        temp_json = json.dumps(temp_dict)

        return temp_json

    def getAnswer(self, question_bank, question_id, answer):
        sock = connect_to_server(self.QB_IP, QB_PORT)
        server_address = (self.QB_IP, QB_PORT)

        header = "mc_answer"  # THIS TELLS THE QB WHAT TYPE OF MESSAGE IT IS AND WHAT TO DO
        header_len = len(header)
        # Pack the header length as a 4-byte integer in network byte order
        header_len_bytes = struct.pack("!I", header_len)

        message = "{}={}".format(question_id, answer)
        data = header_len_bytes + header.encode() + message.encode()

        sock.sendto(data, server_address)  # TCP Should be reliable so don't think we need a check on this.
        response = sock.recv(1024)  # Awaits a response.
        msg = str(response, 'utf-8')
        if (msg == 'T'):
            print(f"Received response: '{msg}', answer was CORRECT")
        else:
            print(f"Received response: '{msg}', answer was INCORRECT")

        return



def connect_to_server(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (host, port)
    try:
        sock.connect(server_address)
        # print("Connection successful!")
    except socket.error as e:
        print(f"Error connecting to server: {e}")
        exit(1)
    return sock