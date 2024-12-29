# SIF Crypto Infrastructure

Manages the connections to crypto exchanges, and deals with everything related to receiving live market data and executing requested orders. The system contains a C++ Backend Component, and a Python Client Component. The client component is able to send requests to the backend through IPC Sockets.

## Build
To build the C++ side of the system, run the following code.
```
make core
```
After doing do, run `./core` on one terminal process to have a live running backend component. The python component is called `CryptoConnection` in a file called `client.py`. Read the below to see the guide on how to use the client class.

### Supported Exchange List:
- Coinbase

