import argparse
from client import CryptoConnection
import time
from datetime import datetime

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Crypto data fetcher")
    parser.add_argument("symbol", type=str, help="Cryptocurrency symbol (e.g., ETH)")
    args = parser.parse_args()

    crypto_symbol = args.symbol

    connection = CryptoConnection()
    connection.connect()

    connection.subscribe(crypto_symbol)

    for x in range(30):
        print(datetime.now())
        data = connection.parse(crypto_symbol)

        time.sleep(1.5)

    connection.unsubscribe(crypto_symbol)
