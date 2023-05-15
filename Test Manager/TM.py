import os
import json
from http.server import BaseHTTPRequestHandler, HTTPServer
import webbrowser  # for opening browser

# local imports
import studentRecords as records
import tester

# constants
QB_PORT = 9001
HTML_LOGIN_FILENAME = "login.html"
HTML_TEST_FILENAME = "test.html"
JSON_FILENAME = "student_info.json"

active_tests = {}


def main():
    global QB_HOST
    ip = input("Enter IP address: ")
    QB_HOST = ip

    webbrowser.open_new('http://localhost:9000')  # open in new window

    # Set up the HTTP server to listen on port 9000
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    server_address = ('', 9000)
    httpd = HTTPServer(server_address, HTTPRequestHandler)

    httpd.serve_forever()  # anything past this won't be run


#################################################
# Dealing with networking with the Browser
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

        if data[0:9] == "username=":  # need to change
            button = data.split('&')[-1].split('=')[1]
            message = data.split('&')[0:-1]

            if button == "Login":
                username = message[0].split('=')[1]
                password = message[1].split('=')[1]

                # Check if the username and password are correct
                if records.checkLogin(username, password):
                    processLogin(username)

                    self.send_response(302)
                    self.send_header('Location', '/test')
                    self.end_headers()

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

        else:
            json_data = json.loads(data)

            # For grabbing unique question for username
            username = json_data['username']

            send_response = True
            if json_data['action'] == 'next':
                response = active_tests[username].nextQuestion()

            elif json_data['action'] == 'back':
                response = active_tests[username].previousQuestion()

            elif json_data['action'] == 'info':
                username = json_data['username']
                response = self.generate_student_info(username)

            elif json_data["action"] == 'test_info':
                username = json_data['username']
                response = json.dumps({"num_questions": active_tests[username].getNumQuestions()})


            elif json_data['action'] == 'submit':
                if active_tests[username].getAnswer(active_tests[username], active_tests[username].getCurrentQuestionNum(),
                                                    json_data["answer"]):
                    response = "correct"
                else:
                    response = "incorrect"
            else:
                send_response = False
                response = 0

            if send_response:
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                print(response)
                self.wfile.write(bytes(response, 'utf-8'))

    def generate_student_info(self, student_id):
        if student_id in records.readRecords():
            student = records.getStudent(student_id)
            name = student['name']
            grade = student['grade']
            temp_dict = {
                "name": name,
                "grade": grade
            }
            temp_json = json.dumps(temp_dict)
            return temp_json
        else:
            return None


def processLogin(student_id):
    print("processing login")
    print("created test object for student_id {}".format(student_id))
    test_obj = tester.Test(student_id, QB_HOST, resume_state=False, num_questions=5)
    active_tests[student_id] = test_obj


if __name__ == '__main__':
    main()
