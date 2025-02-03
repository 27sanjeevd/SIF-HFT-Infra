import socket
import struct
import threading
import errno
from dataclasses import dataclass

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
    
    def unsubscribe(self, msg_type):
        continue_channel = False

        message = struct.pack("!?", continue_channel)
        self.socket.sendall(message)

    def subscribe(self, msg_type):
        self.data[msg_type] = None
        self.running = True

        server_thread = threading.Thread(
            target=self._listen, 
            args=(msg_type,),
            daemon=True
        )
        server_thread.start()

    def _listen(self, msg_type, num_levels = 1):
        continue_channel = True
        message = struct.pack("!?II", continue_channel, self.currency_id[msg_type], num_levels)
        self.socket.sendall(message)

        while self.running:
            try:
                header = self.socket.recv(4)
                if not header:
                    print("Client disconnected")
                    return
                
                message_length = struct.unpack('>i', header)[0]
                market_data = self.socket.recv(message_length)
                if not market_data:
                    print("Client disconnected")
                    return
                
                self.data[msg_type] = market_data

            except socket.error as e:
                if e.errno != errno.EAGAIN and e.errno != errno.EWOULDBLOCK:
                    print(f"Error with client: {e}")
                    return
                continue
            except Exception as e:
                print(f"Error with client: {e}")
                return


    def parse(self, msg_type):
        data = self.data.get(msg_type)
        if not data:
            return None
        
        #if msg_type == "BTC" or msg_type == "ETH":
        values = struct.unpack("!dddd", data[:32])
        market_data = MarketData(*values)
        
        print("Best Bid and Best Ask Information:")
        print(f"Best Bid Price: {market_data.best_bid_price}")
        print(f"Best Bid Volume: {market_data.best_bid_volume}")
        print(f"Best Ask Price: {market_data.best_ask_price}")
        print(f"Best Ask Volume: {market_data.best_ask_volume}")
        print("------------")
        
        return market_data

    def __del__(self):
        self.socket.close()
