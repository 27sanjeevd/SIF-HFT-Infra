# SIF Crypto Infrastructure

Manages the connections to crypto exchanges, and deals with everything related to receiving live market data and executing requested orders. The system contains a C++ Backend Component, and a Python Client Component. The client component is able to send requests to the backend through IPC Sockets.

## Build
To build the C++ side of the system, run the following code.
```
make core
```
After doing so, run `./core` on one terminal process to have a live running backend component. The python component is called `CryptoConnection` in a file called `client.py`. Read the below to see the guide on how to use the client class.

## Client
The client class is able to connect to the core backend through ipc sockets. Here's example python code showcase utilizing the class. 

```
connection = CryptoConnection()
connection.connect()

best_bid, best_ask = connection.get_bbo("eth")

bids, asks = connection.get_best_book("eth", 5)
```

You can view an example file directly with `main.py`.

### Supported Requests
`.get_bbo(currency)`: calling this returns a pair of doubles representing the best bid and best ask amongst all the exchanges supported.
```
best_bid, best_ask = connection.get_bbo("eth")
```

`.get_best_book(currency, num_levels)`: calling this returns the coalesced best book amongst the supported exchanges, up to n levels. Returns a pair of lists, each holding a tuple representing each level. The value in the tuple take the form `(Price, Volume, Num Orders)`.
```
bids, asks = connection.get_best_book("eth", 5)
```


## Supported Exchange List (Receiving Data, Sending Orders):
- Coinbase (✅, ❌)


## Supported Currency List
ETH, BTC, XRP, SOL, DOGE, ADA


## Rate Limits
Note the rate limits for all the supported exchanges.

Coinbase Data: 10 requests per second
