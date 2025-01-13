from client import CryptoConnection

import time

if __name__ == "__main__":
    connection = CryptoConnection()
    connection.connect()

    for x in range(10):
        print(connection.get_bbo("eth"))

        time.sleep(1)