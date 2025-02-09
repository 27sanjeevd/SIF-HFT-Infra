import socket
import struct
import threading
import errno
from dataclasses import dataclass

import time

@dataclass
class MarketData:
    best_bid_price: float
    best_bid_volume: float
    best_ask_price: float
    best_ask_volume: float

class CryptoConnection:
    def __init__(self, host="127.0.0.1", port=8080):
        self.host = host
        self.port = port
        self.exchanges = {"coinbase"}
        
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        self.exchange_id = {"coinbase": 0}
        self.currency_id = {"BTC": 1, "ETH": 2, "XRP": 3, "SOL": 4, "DOGE": 5}
        self.data = {}

    def connect(self):
        try:
            self.socket.connect((self.host, self.port))
            self.socket.setblocking(0)

            print(f"Connected to server at {self.host}:{self.port}")
        except ConnectionError as e:
            print(f"Failed to connect: {e}")
            raise

        self.running = True

        server_thread = threading.Thread(
            target=self._listen, 
            daemon=True
        )
        server_thread.start()
    
    def unsubscribe(self, currency_name):
        continue_channel = 2

        message = struct.pack("!II", continue_channel, self.currency_id[currency_name])
        self.socket.sendall(message)

    def subscribe(self, currency_name, num_levels = 1):
        self.data[self.currency_id[currency_name]] = None

        continue_channel = 1
        message = struct.pack("!III", continue_channel, self.currency_id[currency_name], num_levels)
        self.socket.sendall(message)

    def _listen(self):
        while self.running:
            try:
                currency_id = self.socket.recv(4)
                if not currency_id:
                    print("Server disconnected")
                    return
                
                currency_id = struct.unpack('>i', currency_id)[0]

                header = self.socket.recv(4)

                message_length = struct.unpack('>i', header)[0]
                market_data = self.socket.recv(message_length)
                if not market_data:
                    print("Server disconnected")
                    return

                self.data[currency_id] = market_data

            except socket.error as e:
                if e.errno != errno.EAGAIN and e.errno != errno.EWOULDBLOCK:
                    print(f"Error with Server: {e}")
                    return
                continue
            except Exception as e:
                print(f"Error with Server: {e}")
                return


    def parse(self, currency_name):
        data = self.data.get(self.currency_id[currency_name])
        if not data:
            return None
        
        values = struct.unpack("!dddd", data[:32])
        market_data = MarketData(*values)
        
        print("Best Bid and Best Ask Information:")
        print(f"Best Bid Price: {market_data.best_bid_price}")
        print(f"Best Bid Volume: {market_data.best_bid_volume}")
        print(f"Best Ask Price: {market_data.best_ask_price}")
        print(f"Best Ask Volume: {market_data.best_ask_volume}")
        print("------------")
        
        return market_data
    
    def close(self):
        self.running = False

        continue_channel = 0
        message = struct.pack("!I", continue_channel)
        self.socket.sendall(message)

        if self.socket:
            self.socket.close()

    def __del__(self):
        if self.socket:
            self.socket.close()
