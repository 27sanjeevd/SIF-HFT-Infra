from client import CryptoConnection

import time
from datetime import datetime

if __name__ == "__main__":
    connection = CryptoConnection()
    connection.connect()

    connection.subscribe("ETH")

    for x in range(30):
        print(datetime.now())
        data = connection.parse("ETH")

        time.sleep(1.5)

    connection.unsubscribe("ETH")

    connection.cleanup()