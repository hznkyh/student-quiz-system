# module imports
import os
import json
from http.server import BaseHTTPRequestHandler, HTTPServer
import webbrowser

# local imports
import studentRecords as records
import tester

# constants
HTML_LOGIN_FILENAME = "login.html"
HTML_TEST_FILENAME = "test.html"
JSON_FILENAME = "student_info.json"

# to store all active tests, allows for distinguishing concurrent users
active_tests = {}


def main():
    """
    Main function
    """

    # global variable defined in main due to python assuming a local variable if accesses from the default global scope
    global QB_HOST

    # user has to enter IP address of the QB
    ip = input("Enter IP address:\n> ")
    QB_HOST = ip

    #user also has to enter port number of QB
    port = int(input("Enter port:\n> "))
    tester.QB_PORT = port


    # opens browser in new window/tab (depends on system default browser)
    webbrowser.open_new('http://localhost:9000')

    # Set up the HTTP server to listen on port 9000
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    server_address = ('', 9000)
    httpd = HTTPServer(server_address, HTTPRequestHandler)

    httpd.serve_forever()
    # anything past this won't be run


class HTTPRequestHandler(BaseHTTPRequestHandler):
    """
    This does all the dealing with the networking between client web browser and TM
    """

    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()

            # displays the login page
            with open('login.html', 'r') as f:
                content = f.read()

            self.wfile.write(bytes(content, 'utf-8'))

        elif self.path == '/test':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()

            # displays the main test page
            with open('test.html', 'r') as f:
                content = f.read()

            self.wfile.write(bytes(content, 'utf-8'))

        elif self.path == '/error':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()

            # displays the error page
            with open('error.html', 'r') as f:
                content = f.read()

            self.wfile.write(bytes(content, 'utf-8'))

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        message_data = self.rfile.read(content_length)

        # decodes the message data in utf-8 format
        data = message_data.decode('utf-8')

        # runs if the TM detects that the request sent was a login request
        if data.split('&')[-1] == "Login=Login":
            message = data.split('&')[0:-1]

            # derives the username and password from the login form
            username = message[0].split('=')[1]
            password = message[1].split('=')[1]

            # Check if the username and password are correct
            if records.check_login(username, password):
                connection = process_login(username)
                # Check if connection if successful
                if connection:
                    # sends response to update the page to the test pages
                    self.send_response(302)
                    self.send_header('Location', '/test')
                    self.end_headers()
                else:
                    # sends response to update the page to the error page
                    self.send_response(302)
                    self.send_header('Location', '/error')
                    self.end_headers()

            else:
                # Send a response to the client indicating that the login credentials are incorrect
                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()

                # response message sent to user (can be changed to whatever)
                response_message = "Incorrect login credentials"
                self.wfile.write(bytes(response_message, "utf8"))

                # reloads login page
                with open('login.html', 'r') as f:
                    content = f.read()
                self.wfile.write(bytes(content, "utf8"))

        # runs if the request sent was not a login request and hence must be an AJAX request from the test page
        else:
            # converts the json data sent in ajax request to a python dict
            json_data = json.loads(data)

            # For grabbing unique question for username
            username = json_data['username']

            # checks the json data action field and decides what to do based on what the webpage has requested

            # next question
            if json_data['action'] == 'next':
                response = active_tests[username].next_question()

            # previous question
            elif json_data['action'] == 'back':
                response = active_tests[username].previous_question()

            # student info
            elif json_data['action'] == 'info':
                response = generate_student_info(username)

            # test info
            elif json_data["action"] == 'test_info':
                response = json.dumps({"num_questions": active_tests[username].get_num_questions()})

            # latest attempts
            elif json_data['action'] == 'attempts':
                question_num = active_tests[username].get_current_questionNum()
                response = records.remaining_attempts(username, question_num)

            # test finished
            elif json_data['action'] == 'finished':
                print("FINISHED TEST")
                records.set_test_active_state(username, False)
                response = str(records.get_grade(username) / 0.15)

            # submit question
            elif json_data['action'] == 'submit':
                # Retrieve the remaining attempts for the current question
                question_num = active_tests[username].get_current_questionNum()
                attempts = int(records.remaining_attempts(username, question_num))
                print(f"SENDING: {json_data['answer']}")
                # if the student submitted the correct answer
                print(active_tests[username].get_answer(active_tests[username],
                                                     active_tests[username].get_current_questionNum(),
                                                     json_data["answer"]))
                if (active_tests[username].get_answer(active_tests[username],
                                                     active_tests[username].get_current_questionNum(),
                                                     json_data["answer"])):

                    response = "correct"

                    # Update grade
                    grade = int(records.get_grade(username))
                    records.set_grade(username, grade + attempts)

                    # Set remaining attempts to 0
                    records.set_remaining_attempts(username, question_num, "0")
                else:
                    # If there are no remaining attempts
                    if attempts == 1:
                        response = "No more attempts left. "
                        correct_answer = active_tests[username].get_correct_answer(active_tests[username], active_tests[
                            username].get_current_questionNum())
                        #   If it's answer for programming question
                        if correct_answer[1:5] == "void" or correct_answer[1:4] == "def":
                            response += f"The sample solution is: {correct_answer}"
                        else:
                            response += f"The correct answer is: {correct_answer}"
                        records.set_remaining_attempts(username, question_num, str(attempts - 1))

                    # if the user has already been told they have no more remaining attempts
                    elif attempts == False:
                        response = "Nothing has changed sorry, no more attempts left. "
                        correct_answer = active_tests[username].get_correct_answer(active_tests[username], active_tests[
                            username].get_current_questionNum())
                        #   If it's answer for programming question
                        if correct_answer[1:5] == "void" or correct_answer[1:4] == "def":
                            response += f"The sample solution is: {correct_answer}"
                        else:
                            response += f"The correct answer is: {correct_answer}"

                    # If there are remaining attempts, decrement the attempts and send the response
                    else:
                        records.set_remaining_attempts(username, question_num, str(attempts - 1))
                        response = "incorrect"

            # shouldn't get to here
            else:
                response = 0

            # send the response back to the webpage
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            self.wfile.write(bytes(response, 'utf-8'))


def generate_student_info(student_id):
    """
    Generates the student info from existing records stored in the json file
    @param student_id:
    @return: json file if student exists, None otherwise
    """
    if student_id in records.read_records():
        student = records.get_student(student_id)
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


def process_login(student_id):
    """
    Processes the student login and creates test object for the student
    @param student_id:
    @return: n/a
    """
    print("Processing login...")
    print("Creating test object for student {}".format(student_id))
    # creates test object
    test_obj = tester.Test(student_id, QB_HOST)
    # checks if test object was created successfully, if not, returns false
    if not test_obj.questions:
        return False
    else:
        print("Created test object for student {}".format(student_id))
        # adds text object to active tests dict
        active_tests[student_id] = test_obj
        return True


if __name__ == '__main__':
    main()
