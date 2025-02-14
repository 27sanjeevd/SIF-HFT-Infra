#include "../../include/websockets/coinbase_ws.hpp"

Coinbase_WS::Coinbase_WS(std::shared_ptr<Orderbook> new_book, std::string &ws_id, 
    std::shared_ptr<std::mutex> mutex) : WebsocketConnection(new_book, ws_id, mutex) {
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


void Coinbase_WS::HandleMessage(const std::string_view& message) {
    try {
        simdjson::padded_string padded_message{message.data(), message.size()};
        auto doc = json_parser_.iterate(padded_message);
        auto events = doc["events"];

        std::vector<std::tuple<std::string_view, double, double>> aggregated_updates;

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
                std::string_view side, event_time, price_level_str, new_quantity_str;
                
                bool success = update["side"].get(side) == simdjson::SUCCESS &&
                            update["event_time"].get(event_time) == simdjson::SUCCESS &&
                            update["price_level"].get(price_level_str) == simdjson::SUCCESS &&
                            update["new_quantity"].get(new_quantity_str) == simdjson::SUCCESS;
                
                if (!success) {
                    continue;
                }

                double price = 0.0, volume = 0.0;
                if (!ConvertToDouble(price_level_str, price)) {
                    continue;
                }
                if (!ConvertToDouble(new_quantity_str, volume)) {
                    continue;
                }

                aggregated_updates.emplace_back(side, price, volume);
            }
        }

        {
            std::lock_guard<std::mutex> lock(*mutex_);
            for (const auto& [side, price, volume] : aggregated_updates) {
                if (side == "bid") {
                    curr_book_->update_bid(id_, price * (1 - fee_percentage_), volume);
                } else {
                    curr_book_->update_ask(id_, price * (1 + fee_percentage_), volume);
                }
            }
        }
    } 
    catch (const simdjson::simdjson_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

std::string Coinbase_WS::GetHost() const {
    return host_;
}