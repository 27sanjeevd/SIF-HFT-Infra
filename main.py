from client import CryptoConnection

import time

if __name__ == "__main__":
    connection = CryptoConnection()
    connection.connect()

    result = connection.get_bbo("eth")
    if result:
        best_bid, best_ask = result
        print(best_bid, best_ask)
    else:
        print("failed best_bbo")


    result = connection.get_best_book("xrp", 5)
    if result:
        bids, asks = result

        print(bids)
        print("*************")
        print(asks)
    else:
        print("failed best book")