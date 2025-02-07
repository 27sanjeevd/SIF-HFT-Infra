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


void Crypto_WS::HandleMessage(const std::string& message) {
    try {
        simdjson::padded_string padded_message{message};
        auto doc = json_parser_.iterate(padded_message);
        
        auto result = doc["result"];
        
        std::string_view channel;
        result["channel"].get(channel);

        auto process_orders = [](simdjson::simdjson_result<simdjson::ondemand::value> orders, 
                        std::shared_ptr<Orderbook>& curr_book,
                        const std::string& id, 
                        const double fee_percentage,
                        bool is_bid) {
            for (auto order : orders) {
                std::string_view price_level, level_size, num_orders;
                size_t index = 0;
                
                for (auto value : order) {
                    switch(index) {
                        case 0: value.get(price_level); break;
                        case 1: value.get(level_size); break;
                        case 2: value.get(num_orders); break;
                    }
                    index++;
                }

                double price = std::stod(std::string(price_level));
                double volume = std::stod(std::string(level_size));

                if (is_bid) {
                    curr_book->update_bid(id, price * (1 - fee_percentage), volume);
                }
                else {
                    curr_book->update_ask(id, price * (1 + fee_percentage), volume);
                }
            }
        };


        if (channel == "book") {
            for (auto data : result["data"]) {
                std::lock_guard<std::mutex> lock(*mutex_);
                process_orders(data["asks"], curr_book_, id_, fee_percentage_, false);
                process_orders(data["bids"], curr_book_, id_, fee_percentage_, true);
            }
        }
        else if (channel == "book.update") {
            for (auto data : result["data"]) {
                auto update = data["update"];

                std::lock_guard<std::mutex> lock(*mutex_);
                process_orders(update["asks"], curr_book_, id_, fee_percentage_, false);
                process_orders(update["bids"], curr_book_, id_, fee_percentage_, true);
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