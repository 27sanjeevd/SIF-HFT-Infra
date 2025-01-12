from client import CryptoConnection

if __name__ == "__main__":
    connection = CryptoConnection()
    connection.connect()

    best_bid, best_ask = connection.get_bbo("eth")

    print(best_bid, best_ask)