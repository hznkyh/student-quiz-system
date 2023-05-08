import socket
import os
from http.server import BaseHTTPRequestHandler, HTTPServer
import webbrowser # for opening browser

QB_HOST = 'localhost'
QB_PORT = 9001


# below code is to automatically open a browser window
# only tested on mac, path might need adjustment for windows
file_name = "test.html"

cwd = os.getcwd()
path = cwd + "/" + file_name
url = "file://" + path

webbrowser.open_new(url)  # open in new window


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
            if username == 'myusername' and password == 'mypassword':
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

        elif button == "Submit":
            # Send the data to the QB server using UDP
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
                s.sendto(message_data, (QB_HOST, QB_PORT))

            # Send a response to the client
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            response_message = "Message sent to QB.c server: {}".format(message)
            self.wfile.write(bytes(response_message, "utf8"))

if __name__ == '__main__':
    # Set up the HTTP server to listen on port 9000
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    server_address = ('', 9000)
    httpd = HTTPServer(server_address, HTTPRequestHandler)
    print('Listening on port 9000...')
    httpd.serve_forever()
