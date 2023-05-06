import socket
import os
from http.server import BaseHTTPRequestHandler, HTTPServer

QB_HOST = 'localhost'
QB_PORT = 9001

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
        if self.path == '/':
            content_length = int(self.headers['Content-Length'])
            message_data = self.rfile.read(content_length)
            message = message_data.decode('utf-8')
            username, password = message.split('&')
            username = username.split('=')[1]
            password = password.split('=')[1]

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
        elif self.path == '/test':
            content_length = int(self.headers['Content-Length'])
            message_data = self.rfile.read(content_length)
            message = message_data.decode('utf-8')

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
