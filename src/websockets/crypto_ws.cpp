#include "../../include/websockets/crypto_ws.hpp"

Crypto_WS::Crypto_WS(std::shared_ptr<Orderbook> new_book, std::string &ws_id, 
    std::shared_ptr<std::mutex> mutex) : WebsocketConnection(new_book, ws_id, mutex) {
    Initialize();

    id_to_currency_[1] = "book.BTCUSD-PERP.50";
    id_to_currency_[2] = "book.ETHUSD-PERP.50";
    id_to_currency_[3] = "book.XRPUSD-PERP.50";
    id_to_currency_[4] = "book.SOLUSD-PERP.50";
    id_to_currency_[5] = "book.DOGEUSD-PERP.50";
}

void Crypto_WS::SubscribeToChannel(std::string &currency_name, std::string &channel_name) {
    boost::json::object params = {
        {"channels", boost::json::array{currency_name}},
        {"book_subscription_type", channel_name},
        {"book_update_frequency", 10}
    };

    boost::json::object subscribe_message = {
        {"id", 1},
        {"method", "subscribe"},
        {"params", params}
    };

    std::string msg = boost::json::serialize(subscribe_message);
    ws_.write(net::buffer(msg));
}

std::optional<std::string> Crypto_WS::GetCurrencyName(uint32_t currency_id) {
    if (id_to_currency_.contains(currency_id)) {
        return id_to_currency_[currency_id];
    }

    return std::nullopt;
}


void Crypto_WS::HandleMessage(const std::string_view& message) {
    try {
        simdjson::padded_string padded_message{message.data(), message.size()};
        auto doc = json_parser_.iterate(padded_message);
        auto result = doc["result"];

        std::vector<std::tuple<bool, double, double>> aggregated_updates;
        
        std::string_view channel;
        result["channel"].get(channel);

        auto process_orders = [this](simdjson::simdjson_result<simdjson::ondemand::value> orders, 
                               std::vector<std::tuple<bool, double, double>>& update_list,
                               std::shared_ptr<Orderbook>& curr_book,
                               const std::string& id, 
                               const double fee_percentage,
                               bool is_bid) {
            
            for (auto order : orders) {
                std::string_view price_level_str, new_quantity_str, num_orders_str;
                size_t index = 0;

                for (auto value : order) {
                    switch(index) {
                        case 0: value.get(price_level_str); break;
                        case 1: value.get(new_quantity_str); break;
                        case 2: value.get(num_orders_str); break;
                    }
                    index++;
                }

                double price = 0.0, volume = 0.0;
                if (!ConvertToDouble(price_level_str, price)) {
                    continue;
                }
                if (!ConvertToDouble(new_quantity_str, volume)) {
                    continue;
                }

                update_list.emplace_back(is_bid, price, volume);
            }
        };


        if (channel == "book") {
            for (auto data : result["data"]) {
                std::lock_guard<std::mutex> lock(*mutex_);
                process_orders(data["asks"], aggregated_updates, curr_book_, id_, fee_percentage_, false);
                process_orders(data["bids"], aggregated_updates, curr_book_, id_, fee_percentage_, true);
            }
        }
        else if (channel == "book.update") {
            for (auto data : result["data"]) {
                auto update = data["update"];

                std::lock_guard<std::mutex> lock(*mutex_);
                process_orders(update["asks"], aggregated_updates, curr_book_, id_, fee_percentage_, false);
                process_orders(update["bids"], aggregated_updates, curr_book_, id_, fee_percentage_, true);
            }
        }

        {
            std::lock_guard<std::mutex> lock(*mutex_);
            for (const auto& [side, price, volume] : aggregated_updates) {
                if (side) {
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

std::string Crypto_WS::GetHost() const {
    return host_;
}

std::string Crypto_WS::GetTarget() const {
    return target_;
}