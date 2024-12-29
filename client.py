import socket

class CryptoConnection:


    def __init__(self):
        self.exchanges = {"coinbase"}
        self.host = "127.0.0.1"
        self.port = 8080

    def run(self):
        print("asd")

        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        client_socket.connect((self.host, self.port))
        print(f"Connected to server at {self.host}:{self.port}")

        client_socket.close()


if __name__ == "__main__":
    connection = CryptoConnection()

    connection.run()