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
    connection.subscribe("DOGE")

    time.sleep(2)
    data = connection.parse("DOGE")

    connection.unsubscribe("DOGE")
    connection.unsubscribe(crypto_symbol)

    time.sleep(5)
    connection.close()
