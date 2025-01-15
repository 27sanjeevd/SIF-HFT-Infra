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

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class CoinbaseAdvancedTradeWS {
private:
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};
    const std::string host = "advanced-trade-ws.coinbase.com";
    simdjson::ondemand::parser json_parser;

    void handle_message(const std::string& message) {
        try {
            simdjson::padded_string padded_message{message};
            auto doc = json_parser.iterate(padded_message);

            // Extract "channel"
            std::string_view channel;
            if (doc["channel"].get(channel) == simdjson::SUCCESS) {
                std::cout << "Channel: " << channel << std::endl;
            }

            // Extract "timestamp"
            std::string_view timestamp;
            if (doc["timestamp"].get(timestamp) == simdjson::SUCCESS) {
                std::cout << "Timestamp: " << timestamp << std::endl;
            }

            // Process "events"
            for (auto event : doc["events"]) {
                std::string_view type;
                if (event["type"].get(type) == simdjson::SUCCESS) {
                    // Indicate event type
                    std::cout << "Event Type: " << type << std::endl;

                    // Handle "snapshot" or "update" types
                    if (type == "snapshot" || type == "update") {
                        // Print whether it's a snapshot or update
                        std::cout << "Processing " << type << " event" << std::endl;

                        // Extract product_id
                        std::string_view product_id;
                        if (event["product_id"].get(product_id) == simdjson::SUCCESS) {
                            std::cout << "Product ID: " << product_id << std::endl;
                        }

                        // Process updates
                        for (auto update : event["updates"]) {
                            std::string_view side;
                            std::string_view event_time;
                            std::string_view price_level;
                            std::string_view new_quantity;

                            if (update["side"].get(side) == simdjson::SUCCESS &&
                                update["event_time"].get(event_time) == simdjson::SUCCESS &&
                                update["price_level"].get(price_level) == simdjson::SUCCESS &&
                                update["new_quantity"].get(new_quantity) == simdjson::SUCCESS) {
                                std::cout << "  Side: " << side
                                        << ", Event Time: " << event_time
                                        << ", Price Level: " << price_level
                                        << ", New Quantity: " << new_quantity
                                        << std::endl;
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
        if (!SSL_set_tlsext_host_name(
                ws.next_layer().native_handle(),
                host.c_str())) {
            throw boost::system::system_error(
                static_cast<int>(::ERR_get_error()),
                boost::asio::error::get_ssl_category());
        }
    }

    void connect() {
        try {
            // Resolve and connect
            tcp::resolver resolver{ioc};
            auto const results = resolver.resolve(host, "443");
            auto ep = net::connect(ws.next_layer().next_layer(), results);

            // SSL handshake
            ws.next_layer().handshake(ssl::stream_base::client);

            // WebSocket handshake
            ws.handshake(host, "/ws");

            // Create subscription message
            boost::json::object subscribe_message = {
                {"type", "subscribe"},
                {"product_ids", boost::json::array{"BTC-USD"}},
                {"channel", "level2"}
            };

            // Send subscription message
            std::string msg = boost::json::serialize(subscribe_message);
            std::cout << "Sending subscription message: " << msg << std::endl;
            ws.write(net::buffer(msg));

            // Read messages loop
            beast::flat_buffer buffer;
            while (true) {
                ws.read(buffer);
                std::string message(static_cast<const char*>(buffer.data().data()), buffer.data().size());
                handle_message(message);
                buffer.consume(buffer.size());
            }
        } catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void disconnect() {
        try {
            ws.close(websocket::close_code::normal);
        } catch (std::exception const& e) {
            std::cerr << "Error closing connection: " << e.what() << std::endl;
        }
    }
};

int main() {
    CoinbaseAdvancedTradeWS client;
    client.connect();
    return 0;
}
