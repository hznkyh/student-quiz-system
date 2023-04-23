import socket
import http.server
import socketserver
import os
from urllib.parse import urlparse, parse_qs

PORT = 5500

class MyHttpRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.path = 'test.html'
        return http.server.SimpleHTTPRequestHandler.do_GET(self)

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        params = parse_qs(body.decode())
        answers = {k: v[0] for k, v in params.items()}
        print(answers)
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(b'<html><body><h1>Answers submitted successfully!</h1></body></html>')

        #testing
        url = 'QB address'  # Replace with desired URL
        data = answers  # Replace with desired data to be posted
        data = data.encode('utf-8')
        headers = {'Content-Type': 'application/x-www-form-urlencoded', 'Content-Length': len(data)}
        response = urllib.request.urlopen(url, data=data, headers=headers)
        self.send_response(response.getcode())
        self.end_headers()

handler_object = MyHttpRequestHandler

print("Server started on port", PORT)

# Change the current working directory to the directory of the script
abspath = os.path.abspath(__file__)
print(abspath)
dname = os.path.dirname(abspath)
print(dname)
os.chdir(dname)

with socketserver.TCPServer(("", PORT), handler_object) as my_server:
    my_server.serve_forever()