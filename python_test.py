"""
import http.client
import json

conn = http.client.HTTPSConnection("api.exchange.coinbase.com")
payload = ''
headers = {
  'Content-Type': 'application/json',
  'User-Agent': 'Mozilla/5.0 (X11; CrOS x86_64 8172.45.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.64 Safari/537.36'
}
conn.request("GET", "/currencies/USD", payload, headers)
res = conn.getresponse()
data = res.read()
print(data.decode("utf-8"))


import http.client
import json

conn = http.client.HTTPSConnection("api.exchange.coinbase.com")
payload = ''
headers = {
  'Content-Type': 'application/json',
  'User-Agent': 'Mozilla/5.0 (X11; CrOS x86_64 8172.45.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.64 Safari/537.36'
}
conn.request("GET", "/products/ETH", payload, headers)
res = conn.getresponse()
data = res.read()
print(data.decode("utf-8"))

"""
import http.client
import json

conn = http.client.HTTPSConnection("api.exchange.coinbase.com")
payload = ''
headers = {
  'Content-Type': 'application/json',
  'User-Agent': 'Mozilla/5.0 (X11; CrOS x86_64 8172.45.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.64 Safari/537.36'
}
conn.request("GET", "/products/XRP-usd/book", payload, headers)
res = conn.getresponse()
data = res.read()
print(data.decode("utf-8"))