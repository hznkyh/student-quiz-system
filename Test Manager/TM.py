import socket
import os
import json
import random
from http.server import BaseHTTPRequestHandler, HTTPServer
import webbrowser # for opening browser
import struct


HTML_LOGIN_FILENAME = "login.html"
HTML_TEST_FILENAME = "test.html"
JSON_FILENAME = "student_info.json"

active_tests = {}


# cwd = os.getcwd()
# path = cwd + "/" + file_name
# url = "file://" + path
QB_PORT = 9001

ip = input("Enter IP address: ")
QB_HOST = ip
webbrowser.open_new('http://localhost:9000')  # open in new window
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


def main():
    # Set up the HTTP server to listen on port 9000
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    server_address = ('', 9000)
    httpd = HTTPServer(server_address, HTTPRequestHandler)
    print('Listening on port 9000...')

    openBrowser()


    # anything past this won't be run
    httpd.serve_forever()


    # TODO

    # check if student is halfway through test

    # if halfway through -> resume

    # if not -> begin new test

    # store answers as need be



#################################################
# Dealing with networking with the QB
#################################################



class HTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()

            with open('login.html', 'r') as f:
                content = f.read()

            self.wfile.write(bytes(content, 'utf-8'))
        elif self.path == '/test':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()

            with open('test.html', 'r') as f:
                content = f.read()

            self.wfile.write(bytes(content, 'utf-8'))

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        message_data = self.rfile.read(content_length)

        data = message_data.decode('utf-8')
        button = data.split('&')[-1].split('=')[1]
        message = data.split('&')[0:-1]

        
        if button == "Login":
            username = message[0].split('=')[1]
            password = message[1].split('=')[1]

            # Check if the username and password are correct
            if checkLogin(username, password):
                self.send_response(302)
                self.send_header('Location', '/test')
                self.end_headers()

                # do all necessary tasks after login
                processLogin(username)

            else:
                # Send a response to the client indicating that the login credentials are incorrect
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                response_message = "Incorrect login credentials"
                self.wfile.write(bytes(response_message, "utf8"))
                with open('login.html', 'r') as f:
                    content = f.read()
                self.wfile.write(bytes(content, "utf8"))

            # now need to create a test instance before the test is displayed

        
        elif button == "Submit":
            
            sock = connect_to_server(QB_HOST, QB_PORT)
            server_address = (QB_HOST, QB_PORT)
            

            header = "question"
            header_len = len(header)
            
            # Pack the header length as a 4-byte integer in network byte order
            header_len_bytes = struct.pack("!I", header_len)
            data = header_len_bytes + header.encode() + message_data

            sock.sendto(data, server_address) #TCP Should be reliable so don't think we need a check on this.
            response = sock.recv(1024) #Awaits a response.
            msg = str(response,'utf-8')
            print(f"Received response: {msg}")

            # Send a response to the client
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            response_message = "Message sent to QB.c server: {}".format(message)
            self.wfile.write(bytes(response_message, "utf8"))


#################################################
# General functions
#################################################

def processLogin(student_id):
    print("processing login")
    # TODO check if test instance already exists

    # once logged in activate a test
    # these are already the defualt parameters, purely for clarity
    active_tests[student_id] = Test(resume_state=False, num_questions=10)
    print("created test object")



#################################################
# Code for test instances
#################################################


# each test can be assigned a session id 
# would allow concurrently running sessions to be distinguished
    
session_ids = []

class Test:
    def __init__(self, resume_state=False, num_questions=10):
        
        if not resume_state:
            self.questions = []
            self.session_id = random.randint(0, 1000000)
            self.question_counter = 0
    
            # create a session id to keep track of the test
            if len(session_ids) == 0:
                self.session_id = 0
            else:
                self.session_id = session_ids[-1] + 1
    
            for _ in range(num_questions):
                # retrieve all of the questions before the user begins the test
                # questions could be sent back to QB to be marked individually though
                pass

        else: # if state is to be resumed
            # read file
            pass


    # For when the next question button is pressed
    def nextQuestion(self):
        if question_counter > num_questions - 1:
            return False

        question_counter += 1
        return self.questions[question_counter]


    # For when the previous question button is pressed
    def previousQuestion(self):
        if question_counter == 0:
            return False

        question_counter -= 1
        return self.questions[question_counter]


    # returns the number of questions in the test session
    def getNumQuestions(self):
        return len(self.questions)


    # save the state of the current question to a file
    def saveState(self):
        pass



#################################################
# Functions for accessing student records 
#################################################


# returns json records as a dict
# key is the student id (as a string)
# each key refers to as a sub dictionary with keys:
#   name (str), grade(int), password(str)

def readRecords():
    with open(JSON_FILENAME, "r") as file:
        data = json.load(file)

        return data


# Probably not really necessary but gives a good basis for how to manage json files
def printStudentSummary():
    with open(JSON_FILENAME, "r") as file:
        json_data = json.load(file)

        print("Student Summary")
        for student_id, student in json_data.items():
            print(student_id, "->", student)


# checks if students login details are correct
def checkLogin(student_id, password):
    json_data = readRecords()

    if student_id in json_data:
        if json_data[student_id]["password"] == password:
            print("Correct login details!")
            print("Logged in as {}".format(json_data[student_id]["name"]))
            return True
        else:
            print("Incorrect login details.")
    else:
        print("Student id not on record")

    return False


# returns info on student in the form of a dictionary
def getStudent(student_id):
    json_data = readRecords()

    name = json_data[student_id]["name"]
    grade = json_data[student_id]["grade"]

    return {
        "name": name,
        "grade": grade
    }

# returns student grade
def getGrade(student_id):
    json_data = readRecords()

    return json_data[student_id]["grade"]


# sets the grade of the student
def setGrade(student_id, new_grade):

    json_data = readRecords()
    json_data[student_id]["grade"] = new_grade

    json_object = json.dumps(json_data, indent=4)

    with open(JSON_FILENAME, "w") as file:
        file.write(json_object)


# resets all grades
def resetGrades():
    json_data = readRecords()

    for student_id in json_data:
        setGrade(student_id, 0)


#################################################
# TODO
#################################################

def resumeTest(student_id):
    pass



#################################################
# Functions for communicating with QB
#################################################

def get_question(question_bank):
    question = question_bank.get_question()

    return question
    

def mark_question(question_bank, answer):
    correct = question_bank.mark_question()

    if correct:
        # adjust mark as necessary
        pass

#################################################
# Opens Broswer
#################################################

def openBrowser():
    print("opening browser...")
    # only tested on mac, path might need adjustment for windows
    cwd = os.getcwd()
    path = cwd + "/" + HTML_LOGIN_FILENAME
    url = "file://" + path
    
    webbrowser.open_new(url)  # open in new window



if __name__ == '__main__':
    main()

