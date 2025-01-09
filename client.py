import socket

class CryptoConnection:


    def __init__(self):
        self.exchanges = {"coinbase"}
        self.host = "127.0.0.1"
        self.port = 8080

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect(self):
        print("connect")

        self.socket.connect((self.host, self.port))
        print(f"Connected to server at {self.host}:{self.port}")

    def get_bbo(self):
        return

    def __del__(self):
        self.socket.close()