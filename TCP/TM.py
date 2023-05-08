import socket
import hashlib
from http.server import BaseHTTPRequestHandler, HTTPServer

QB_HOST = '10.135.187.157'
QB_PORT = 9001


def build_message_with_header(message):
    # Calculate the checksum of the message
    message_checksum = hashlib.sha256(message.encode('utf-8')).hexdigest()

    # Build the header, containing the message type and checksum
    header = bytearray()
    header.append(0x01) # message type 0x01 for example
    header.extend(bytes.fromhex(message_checksum)) # 32-byte SHA-256 checksum

    # Concatenate the header and message into a single byte array
    message_with_header = header + message.encode('utf-8')
    return message_with_header


class HTTPRequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        message_data = self.rfile.read(content_length)
        message = message_data.decode('utf-8')

        # Calculate the checksum of the message
        message_checksum = hashlib.sha256(message_data).hexdigest()

        # Build the header, containing the message type and checksum
        header = bytearray()
        header.append(0x01) # message type 0x01 for example
        header.extend(bytes.fromhex(message_checksum)) # 32-byte SHA-256 checksum

        # Concatenate the header and message into a single byte array
        message_with_header = header + message_data

        # Send the data to the QB server using TCP
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((QB_HOST, QB_PORT))
            s.sendall(message_with_header)
            print("Message Sent")

            # Wait for the response from the server
            response = s.recv(1024)
            print("Response Received: ", response.decode())

        # Send a response to the client
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        response_message = "Message sent to QB.c server: {}".format(message)
        self.wfile.write(bytes(response_message, "utf8"))



if __name__ == '__main__':
    # Create a TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Connect to the QB server
        s.connect((QB_HOST, QB_PORT))
        print('Connected to QB server')

        # Send messages to the QB server
        while True:
            message = input('Enter a message: ')
            if message.lower() == 'q':
                break

            # Build message with header
            message_with_header = build_message_with_header(message)

            # Send message to the QB server
            s.sendall(message_with_header)

            # Receive response from the QB server
            response = s.recv(1024)
            print('Response from QB server:', response.decode('utf-8'))
