#include "../include/websocket.hpp"

WebsocketConnection::WebsocketConnection(Orderbook &new_book, std::string &ws_id) {
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
        throw boost::system::system_error(
            static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
    }

    curr_book_ = new_book;
    id_ = ws_id;

    curr_book_.initialize_exchange(id_);
}

void WebsocketConnection::Connect(std::string &currency_name, std::string &channel_name) {
    try {
        EstablishConnection();
        SubscribeToChannel(currency_name, channel_name);
        StartMessageLoop();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void WebsocketConnection::EstablishConnection() {
    tcp::resolver resolver{ioc_};
    auto const results = resolver.resolve(host_, "443");
    net::connect(ws_.next_layer().next_layer(), results);
    
    ws_.next_layer().handshake(ssl::stream_base::client);
    ws_.handshake(host_, "/ws");
}

void WebsocketConnection::SubscribeToChannel(std::string &currency_name, std::string &channel_name) {
    boost::json::object subscribe_message = {
        {"type", "subscribe"},
        {"product_ids", boost::json::array{currency_name}},
        {"channel", channel_name}
    };

    std::string msg = boost::json::serialize(subscribe_message);
    ws_.write(net::buffer(msg));
}

void WebsocketConnection::StartMessageLoop() {
    beast::flat_buffer buffer;
    
    while (true) {
        ws_.read(buffer);
        ProcessMessage(buffer);
        buffer.consume(buffer.size());
    }
}

void WebsocketConnection::ProcessMessage(const beast::flat_buffer& buffer) {
    std::string message(
        static_cast<const char*>(buffer.data().data()),
        buffer.data().size()
    );
    HandleMessage(message);
}

void WebsocketConnection::Disconnect() {
    try {
        ws_.close(websocket::close_code::normal);
    } catch (std::exception const& e) {
        std::cerr << "Error closing connection: " << e.what() << std::endl;
    }
}

void WebsocketConnection::HandleMessage(const std::string& message) {
    uint32_t bid_count = curr_book_.get_max_levels();
    uint32_t ask_count = curr_book_.get_max_levels();

    try {
        simdjson::padded_string padded_message{message};
        auto doc = json_parser_.iterate(padded_message);
        auto events = doc["events"];

        for (auto event : events) {
            std::string_view type;
            if (event["type"].get(type) != simdjson::SUCCESS) {
                continue;
            }

            if (type != "update" && type != "snapshot") {
                continue;
            }

            auto updates = event["updates"];
            for (auto update : updates) {
                std::string_view side, event_time, price_level, new_quantity;
                
                bool success = update["side"].get(side) == simdjson::SUCCESS &&
                            update["event_time"].get(event_time) == simdjson::SUCCESS &&
                            update["price_level"].get(price_level) == simdjson::SUCCESS &&
                            update["new_quantity"].get(new_quantity) == simdjson::SUCCESS;
                
                if (!success) {
                    continue;
                }
                
                if ((side == "bid" && bid_count == 0) || 
                    (side == "offer" && ask_count == 0)) {
                    continue;
                }

                double price = std::stod(std::string(price_level));
                double volume = std::stod(std::string(new_quantity));

                if (side == "bid") {
                    bid_count--;
                    curr_book_.update_bid(id_, price, volume);
                } else {
                    ask_count--;
                    curr_book_.update_ask(id_, price, volume);
                }
            }
        }
    } catch (const simdjson::simdjson_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}