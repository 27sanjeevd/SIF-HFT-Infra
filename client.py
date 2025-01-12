import socket
import struct

class CryptoConnection:


    def __init__(self):
        self.exchanges = {"coinbase"}
        self.host = "127.0.0.1"
        self.port = 8080

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect(self):

        self.socket.connect((self.host, self.port))
        print(f"Connected to server at {self.host}:{self.port}")

    def get_bbo(self, currency):
        message_name = "best_bbo".ljust(16, '\0')[:16]
        asset_name = currency.ljust(4, '\0')[:4]

        message_size = 20

        message = struct.pack('!I16s4s', message_size, message_name.encode('utf-8'), asset_name.encode('utf-8'))

        self.socket.sendall(message)
        print("send request")

        message = self.socket.recv(20)

        message_size = struct.unpack('!I', message[:4])[0]
        best_bid, best_ask = struct.unpack('!dd', message[4:])
        
        return best_bid, best_ask

    def __del__(self):
        self.socket.close()