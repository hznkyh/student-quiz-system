import socket
from http.server import BaseHTTPRequestHandler, HTTPServer

QB_HOST = 'localhost'
QB_PORT = 9001

class HTTPRequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
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
    server_address = ('', 9000)
    httpd = HTTPServer(server_address, HTTPRequestHandler)
    print('Listening on port 9000...')
    httpd.serve_forever()
