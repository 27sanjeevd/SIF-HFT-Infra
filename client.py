import socket
import struct

class CryptoConnection:


    def __init__(self):
        self.exchanges = {"coinbase"}
        self.host = "127.0.0.1"
        self.port = 8080

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.exchange_id = {}
        self.exchange_id["coinbase"] = 0

    def connect(self):

        self.socket.connect((self.host, self.port))
        print(f"Connected to server at {self.host}:{self.port}")


    def get_bbo(self, currency):
        message_name = "best_bbo".ljust(16, '\0')
        asset_name = currency.ljust(4, '\0')
        message = struct.pack('!I16s4s', 20, message_name.encode(), asset_name.encode())

        self.socket.sendall(message)

        success = struct.unpack(">i", self.socket.recv(4))[0]
        if success == 0:
            return None

        message = self.socket.recv(20)
        offset = 4
        best_bid, best_ask = struct.unpack_from(">dd", message, offset)

        return best_bid, best_ask
        

    def get_best_book(self, currency, num_levels):
        message_name = "best_book".ljust(16, '\0')
        asset_name = currency.ljust(4, '\0')
        message = struct.pack('!I16s4sI', 20, message_name.encode(), asset_name.encode(), num_levels)

        self.socket.sendall(message)

        success = struct.unpack(">i", self.socket.recv(4))[0]
        if success == 0:
            return None

        message = self.socket.recv(8 + num_levels * 48)
        offset = 8
        bids, asks = [], []

        for _ in range(num_levels):
            bid_price, bid_volume, bid_orders = struct.unpack_from("!ddd", message, offset)
            bids.append((bid_price, bid_volume, int(bid_orders)))
            offset += 24

            ask_price, ask_volume, ask_orders = struct.unpack_from("!ddd", message, offset)
            asks.append((ask_price, ask_volume, int(ask_orders)))
            offset += 24

        return bids, asks

    def get_latest_trades(self, currency, exchange, num_trades):
        pass

    def __del__(self):
        self.socket.close()