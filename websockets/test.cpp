#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/json.hpp>
#include <simdjson.h>
#include <iostream>
#include <string>
#include <string_view>

#include <chrono>

#include "include/orderbook.hpp"

using namespace std::chrono;

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

struct OrderUpdate {
    std::string_view side;
    double price_level;
    double new_quantity;
};

class CoinbaseAdvancedTradeWS {
 private:
  net::io_context ioc_;
  ssl::context ctx_{ssl::context::tlsv12_client};
  websocket::stream<beast::ssl_stream<tcp::socket>> ws_{ioc_, ctx_};
  const std::string host_ = "advanced-trade-ws.coinbase.com";
  simdjson::ondemand::parser json_parser_;

  Orderbook curr_book_;


  uint64_t update_counter_ = 0;
  std::chrono::high_resolution_clock::time_point start_;

  uint64_t time_diff_count_ = 0;


    std::vector<OrderUpdate> parse_updates(const char* data, size_t len) {
        std::vector<std::string_view> updates;

        
    }

    void HandleMessages(const char* input_message, size_t len) {
        std::vector<OrderUpdate> results = parse_updates(input_message, len);

        for (auto& result : results) {
            start_ = high_resolution_clock::now();

            if (result.side == "bid") {
                curr_book_.update_bid(result.price_level, result.new_quantity);
            }
            else {
                curr_book_.update_ask(result.price_level, result.new_quantity);
            }

            update_counter_++;
            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);

            time_diff_count_ += static_cast<uint64_t>(duration.count());
            if (update_counter_ % 1000 == 0) {
                std::cout << "Order: " << update_counter_ << " taking avg time: " << time_diff_count_ / update_counter_ << "\n";
            }
        }
    }




  void HandleMessage(const std::string& message) {
    try {
      simdjson::padded_string padded_message{message};
      auto doc = json_parser_.iterate(padded_message);

      std::string_view channel;
      if (doc["channel"].get(channel) == simdjson::SUCCESS) {
        //std::cout << "Channel: " << channel << std::endl;
      }

      std::string_view timestamp;
      if (doc["timestamp"].get(timestamp) == simdjson::SUCCESS) {
       //std::cout << "Timestamp: " << timestamp << std::endl;
      }

      for (auto event : doc["events"]) {
        std::string_view type;
        if (event["type"].get(type) == simdjson::SUCCESS) {
          //std::cout << "Event Type: " << type << std::endl;

          if (type == "update") {
            //std::cout << "Processing " << type << " event" << std::endl;

            std::string_view product_id;
            if (event["product_id"].get(product_id) == simdjson::SUCCESS) {
              //std::cout << "Product ID: " << product_id << std::endl;
            }

            for (auto update : event["updates"]) {
              std::string_view side;
              std::string_view event_time;
              std::string_view price_level;
              std::string_view new_quantity;

              if (update["side"].get(side) == simdjson::SUCCESS &&
                  update["event_time"].get(event_time) == simdjson::SUCCESS &&
                  update["price_level"].get(price_level) == simdjson::SUCCESS &&
                  update["new_quantity"].get(new_quantity) == simdjson::SUCCESS) {

                double price = std::stod(std::string(price_level));
                double volume = std::stod(std::string(new_quantity));

                if (type == "snapshot") {
                    if (side == "bid") {
                        curr_book_.add_bid(price, volume);
                    }
                    else {
                        curr_book_.add_ask(price, volume);
                    }
                }
                else {
                    start_ = high_resolution_clock::now();

                    if (side == "bid") {
                        curr_book_.update_bid(price, volume);
                    }
                    else {
                        curr_book_.update_ask(price, volume);
                    }

                    update_counter_++;
                    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
                    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);

                    time_diff_count_ += static_cast<uint64_t>(duration.count());
                    if (update_counter_ % 1000 == 0) {
                        std::cout << "Order: " << update_counter_ << " taking avg time: " << time_diff_count_ / update_counter_ << "\n";
                    }
                }

                /*
                std::cout << "  Side: " << side
                          << ", Event Time: " << event_time
                          << ", Price Level: " << price_level
                          << ", New Quantity: " << new_quantity << std::endl;
                */
              }

            }
          }
        }
      }
    } catch (const simdjson::simdjson_error& e) {
      std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
  }

 public:
  CoinbaseAdvancedTradeWS() {
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
      throw boost::system::system_error(
          static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
    }
  }

  void Connect() {
    try {
      tcp::resolver resolver{ioc_};
      auto const results = resolver.resolve(host_, "443");
      auto ep = net::connect(ws_.next_layer().next_layer(), results);

      ws_.next_layer().handshake(ssl::stream_base::client);
      ws_.handshake(host_, "/ws");

      boost::json::object subscribe_message = {
          {"type", "subscribe"},
          {"product_ids", boost::json::array{"BTC-USD"}},
          {"channel", "level2"}};

      std::string msg = boost::json::serialize(subscribe_message);
      std::cout << "Sending subscription message: " << msg << std::endl;
      ws_.write(net::buffer(msg));

      beast::flat_buffer buffer;

      while (true) {

        ws_.read(buffer);

        const char* input_data = static_cast<const char*>(buffer.data().data());
        size_t len = buffer.data().size();

        HandleMessages(input_data, len);
        //std::string message(static_cast<const char*>(buffer.data().data()), buffer.data().size());
        //HandleMessage(message);
        buffer.consume(buffer.size());
      }
    } 
    catch (std::exception const& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

  void Disconnect() {
    try {
      ws_.close(websocket::close_code::normal);
    } catch (std::exception const& e) {
      std::cerr << "Error closing connection: " << e.what() << std::endl;
    }
  }
};

int main() {
  CoinbaseAdvancedTradeWS client;
  client.Connect();
  return 0;
}
