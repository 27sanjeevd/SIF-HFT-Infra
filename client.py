import asyncio
import socket
import struct
import threading
from dataclasses import dataclass
from typing import Optional, Set

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
        self.exchanges: Set[str] = {"coinbase"}
        self.running = False
        
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._event_loop: Optional[asyncio.AbstractEventLoop] = None
        self._background_tasks = set()
        self._reader = None
        self._writer = None
        
        self.exchange_id = {"coinbase": 0}
        self.currency_id = {"ETH": 1}
        self.data = {}

    def connect(self):
        try:
            self.socket.connect((self.host, self.port))
            self.socket.setblocking(False)
            print(f"Connected to server at {self.host}:{self.port}")
        except ConnectionError as e:
            print(f"Failed to connect: {e}")
            raise
    
    def unsubscribe(self, msg_type):
        message = struct.pack("!I", 0)
        self.socket.sendall(message)

    def subscribe(self, msg_type):
        self.data[msg_type] = None
        self.running = True
        
        self._event_loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self._event_loop)
        
        thread = threading.Thread(
            target=self._start_background_loop,
            args=(self._event_loop,),
            daemon=True
        )
        thread.start()
        
        self._setup_connection()
        self._start_listener(msg_type)

    def _setup_connection(self):
        future = asyncio.run_coroutine_threadsafe(
            asyncio.open_connection(sock=self.socket),
            self._event_loop
        )
        self._reader, self._writer = future.result()

    def _start_listener(self, msg_type):
        task = asyncio.run_coroutine_threadsafe(
            self._listen(msg_type), 
            self._event_loop
        )
        self._background_tasks.add(task)

    @staticmethod
    def _start_background_loop(loop):
        asyncio.set_event_loop(loop)
        loop.run_forever()

    async def _listen(self, msg_type):
        if not self._writer:
            raise RuntimeError("Writer not initialized")
            
        message = struct.pack("!I", self.currency_id[msg_type])
        self._writer.write(message)
        await self._writer.drain()

        while self.running:
            try:
                await self._read_data(msg_type)
                if not self.running:
                    break
            except Exception as e:
                print(f"Error in listener: {e}")
                self.running = False
                break
            await asyncio.sleep(0)

    async def _read_data(self, msg_type):
        if not self._reader or not self._writer:
            raise RuntimeError("Reader/Writer not initialized")
            
        try:
            size_data = await self._reader.read(4)
            if not size_data:
                await self._handle_disconnection()
                return
            
            remaining_size = struct.unpack('>i', size_data)[0]
            market_data = await self._reader.read(remaining_size)
            self.data[msg_type] = market_data
            
        except (ConnectionResetError, ConnectionAbortedError) as e:
            await self._handle_disconnection(str(e))

    async def _handle_disconnection(self, error_msg="Server disconnected"):
        print(f"{error_msg}, closing connection.")
        self.running = False
        if self._writer:
            self._writer.close()
            await self._writer.wait_closed()

    def parse(self, msg_type):
        """Parse the received market data."""
        data = self.data.get(msg_type)
        if not data:
            return None
        
        if msg_type == "ETH":
            values = struct.unpack("!dddd", data[:32])
            market_data = MarketData(*values)
            
            print("Best Bid and Best Ask Information:")
            print(f"Best Bid Price: {market_data.best_bid_price}")
            print(f"Best Bid Volume: {market_data.best_bid_volume}")
            print(f"Best Ask Price: {market_data.best_ask_price}")
            print(f"Best Ask Volume: {market_data.best_ask_volume}")
            print("------------")
            
            return market_data

    async def cleanup(self):
        self.running = False
        
        if self._background_tasks:
            for task in self._background_tasks:
                task.cancel()
            await asyncio.gather(*self._background_tasks, return_exceptions=True)
        
        if hasattr(self, '_writer') and self._writer:
            self._writer.close()
            await self._writer.wait_closed()
        
        if self.socket:
            self.socket.close()

    def __del__(self):
        if self._background_tasks:
            for task in self._background_tasks:
                if not task.done():
                    task.cancel()
        if self.socket:
            self.socket.close()
