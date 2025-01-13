from client import CryptoConnection

import time

if __name__ == "__main__":
    connection = CryptoConnection()
    connection.connect()

    best_bid, best_ask = connection.get_bbo("eth")
    print(best_bid, best_ask)

    bids, asks = connection.get_best_book("eth", 20)
    print(bids)
    print("*************")
    print(asks)