#include "../../include/websockets/coinbase_ws.hpp"

Coinbase_WS::Coinbase_WS(std::shared_ptr<Orderbook> new_book, std::string &ws_id) : WebsocketConnection(new_book, ws_id) {
    Initialize();

    id_to_currency_[1] = "BTC-USD";
    id_to_currency_[2] = "ETH-USD";
    id_to_currency_[3] = "XRP-USD";
    id_to_currency_[4] = "SOL-USD";
    id_to_currency_[5] = "DOGE-USD";
}

void Coinbase_WS::SubscribeToChannel(std::string &currency_name, std::string &channel_name) {
    boost::json::object subscribe_message = {
        {"type", "subscribe"},
        {"product_ids", boost::json::array{currency_name}},
        {"channel", channel_name}
    };

    std::string msg = boost::json::serialize(subscribe_message);
    ws_.write(net::buffer(msg));
}

std::optional<std::string> Coinbase_WS::GetCurrencyName(uint32_t currency_id) {
    if (id_to_currency_.contains(currency_id)) {
        return id_to_currency_[currency_id];
    }

    return std::nullopt;
}


void Coinbase_WS::HandleMessage(const std::string& message) {
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

                double price = std::stod(std::string(price_level));
                double volume = std::stod(std::string(new_quantity));

                if (side == "bid") {
                    curr_book_->update_bid(id_, price, volume);
                } 
                else {
                    curr_book_->update_ask(id_, price, volume);
                }
            }
        }
    } catch (const simdjson::simdjson_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

std::string Coinbase_WS::GetHost() const {
    return host_;
}