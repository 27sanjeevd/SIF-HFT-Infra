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

        
        success_message = self.socket.recv(4)
        success = struct.unpack_from(">i", success_message, offset=0)[0]

        if success == 0:
            return None
        
        message = self.socket.recv(20)
        offset = 0
        
        message_size = struct.unpack_from(">i", message, offset=offset)
        offset += 4

        best_bid, best_ask = struct.unpack_from(">dd", message, offset=offset)

        return best_bid, best_ask
    
    def get_best_book(self, currency, num_levels):
        message_name = "best_book".ljust(16, '\0')[:16]
        asset_name = currency.ljust(4, '\0')[:4]

        message_size = 20
        message = struct.pack('!I16s4sI', message_size, message_name.encode('utf-8'), asset_name.encode('utf-8'), num_levels)

        self.socket.sendall(message)

        success_message = self.socket.recv(4)
        success = struct.unpack_from(">i", success_message, offset=0)[0]

        if success == 0:
            return None

        message = self.socket.recv(8 + num_levels * 48)
        offset = 0

        message_size, num_levels = struct.unpack_from("!II", message, offset=offset)
        offset += 8

        bids = []
        asks = []

        for _ in range(num_levels):
            bid_price, bid_volume, bid_orders = struct.unpack_from("!ddd", message, offset=offset)
            bids.append((bid_price, bid_volume, int(bid_orders)))
            offset += 24

            ask_price, ask_volume, ask_orders = struct.unpack_from("!ddd", message, offset=offset)
            asks.append((ask_price, ask_volume, int(ask_orders)))
            offset += 24

        return bids, asks

    def __del__(self):
        self.socket.close()