from client import CryptoConnection

import time

if __name__ == "__main__":
    connection = CryptoConnection()
    connection.connect()

    connection.subscribe("ETH")

    for x in range(10):
        connection.parse("ETH")

        time.sleep(1)

    connection.unsubscribe("ETH")

    time.sleep(5)

    connection.cleanup()