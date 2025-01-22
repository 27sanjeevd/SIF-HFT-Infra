# SIF Crypto Infrastructure

Manages the connections to crypto exchanges, and deals with everything related to receiving live market data and executing requested orders. The system contains a C++ Backend Component, and a Python Client Component. The client component is able to send requests to the backend through IPC Sockets.

## Build
To build the C++ side of the system, run the following code.
```
make core
```
After doing so, run `./core` on one terminal process to have a live running backend component. The python component is called `CryptoConnection` in a file called `client.py`. Read the below to see the guide on how to use the client class.

## Client
The client class is able to connect to the core backend through IPC sockets. Here's example python code showcasing using the class. 

```
connection = CryptoConnection()
connection.connect()

connection.subscribe("ETH")

time.sleep(5)
data = connection.parse("ETH")

connection.cleanup()
```

You can view an example file directly with `main.py`.

### Client Logic
When subscribing to a ticker, it tells the server that it can start sending snapshots of the current orderbook of that ticker to the client. To avoid spam, it only sends a snapshot when one of the top 10 levels has been updated.

Receiving and decoding the snapshots happens lazily. The client runs an asynchronous background task monitoring the socket, and blocks to receive only when it has data it can grab. Without being parsed, the snapshot is stored in a data buffer representing the most recent snapshot for that ticker.

Only once the client explicitly calls the `.parse(ticker)` method does the client decode the snapshot stored in the buffer and returns the data. This way, unnecessary computation isn't being spent decoding snapshots that get immediately overwritten when newer data comes in, and ensures the client has access to the most recent snapshot received when it requests that data.

If there is no snapshot available when `parse` is called, then the method returns `None`.

## Supported Exchange List (Receiving Data, Sending Orders):
- Coinbase (✅, ❌)


## Supported Currency List
ETH (default)
