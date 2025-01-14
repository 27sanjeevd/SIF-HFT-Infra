from client import CryptoConnection

import time

if __name__ == "__main__":
    connection = CryptoConnection()
    connection.connect()

    result = connection.get_bbo("eth")
    print(result)

    result = connection.get_bbo("do")
    print(result)

    result = connection.get_best_book("eth", 5)
    print(result)

    result = connection.get_best_book("do", 5)
    print(result)