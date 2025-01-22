#include "../include/orderbook.hpp"

#include <sys/socket.h>
#include <libkern/OSByteOrder.h>

Orderbook::Orderbook() {}

template <typename T>
void Orderbook::rebalance(T& orders_map) {
    
    while (orders_map.size() > MAX_LEVELS) {
        auto it = std::prev(orders_map.end());
        orders_map.erase(it);
    }
}

template <typename T>
void Orderbook::update_level(const std::string& exchange_id, double price, double new_volume,
                    T& orders_map, ExchangeOrderMap& exchanges) {
    
    
    if (new_volume > 0) {
        exchanges[exchange_id][price] = new_volume;
    } 
    else {
        delete_level(exchange_id, price, orders_map, exchanges);
        return;
    }

    double total_volume = get_total_volume_at_price(price, exchanges);

    if (total_volume > 0) {
        orders_map[price] = total_volume;
    } 
    else {
        orders_map.erase(price);
    }

    rebalance(orders_map);
}

template <typename T>
void Orderbook::delete_level(const std::string& exchange_id, double price,
                    T& orders_map, ExchangeOrderMap& exchanges) {
    
    
    if (exchanges.count(exchange_id) != 0) {
        exchanges[exchange_id].erase(price);
    }

    double total_volume = get_total_volume_at_price(price, exchanges);

    if (total_volume > 0) {
        orders_map[price] = total_volume;
    } 
    else {
        orders_map.erase(price);
    }
}

double Orderbook::get_total_volume_at_price(double price, const ExchangeOrderMap& exchanges) const {
    double total_volume = 0;
    for (const auto& [exchange_id, price_map] : exchanges) {
        auto it = price_map.find(price);
        if (it != price_map.end()) {
            total_volume += it->second;
        }
    }
    return total_volume;
}

void Orderbook::send_snapshot() {
    if (top_levels_updated_ || bids_.size() == 0 || asks_.size() == 0 || client_send_list_.size() == 0) {
        return; 
    }
    
    top_levels_updated_ = true;
    
    char buffer[36];

    uint32_t remainingSize = 32;
    double bestBidPrice = bids_.begin()->first;
    double bestBidVolume = bids_.begin()->second;
    double bestAskPrice = asks_.begin()->first;
    double bestAskVolume = asks_.begin()->second;

    remainingSize = OSSwapHostToBigInt32(remainingSize);
    std::memcpy(buffer, &remainingSize, 4);

    ToNetworkOrder(bestBidPrice, buffer + 4);
    ToNetworkOrder(bestBidVolume, buffer + 12);
    ToNetworkOrder(bestAskPrice, buffer + 20);
    ToNetworkOrder(bestAskVolume, buffer + 28);

    
    for (auto client_socket : client_send_list_) {
        send(client_socket, buffer, sizeof(buffer), 0);
    }
}

void Orderbook::ToNetworkOrder(double value, char* buffer) {
    uint64_t raw;
    std::memcpy(&raw, &value, sizeof(raw));
    raw = OSSwapHostToBigInt64(raw);
    std::memcpy(buffer, &raw, sizeof(raw));
}

template <typename T, typename Compare>
bool Orderbook::IsInFirstNKeys(T& orders_map, double price, Compare comp) {
    if (orders_map.size() < TOP_LEVELS) {
        return true;
    }

    typename T::const_iterator it = orders_map.begin();
    std::advance(it, TOP_LEVELS - 1);

    if (comp(price, it->first)) {
        return true;
    }

    return false;
}


void Orderbook::update_bid(const std::string &exchange_id, double price, double new_volume) {
    update_level(exchange_id, price, new_volume, bids_, exchange_bids_);
    //print_bbo();

    if (IsInFirstNKeys(bids_, price, std::greater<double>())) {
        top_levels_updated_ = false;
    }
    send_snapshot();
}

void Orderbook::update_ask(const std::string &exchange_id, double price, double new_volume) {
    update_level(exchange_id, price, new_volume, asks_, exchange_asks_);
    //print_bbo();

    if (IsInFirstNKeys(asks_, price, std::less<double>())) {
        top_levels_updated_ = false;
    }
    send_snapshot();
}

void Orderbook::print_bbo() {

    if (bids_.size() > 0) {
        std::cout << "Bid: " << std::fixed << std::setprecision(4) << bids_.begin()->first << " ";
    }
    if (asks_.size() > 0) {
        std::cout << "Ask: " << std::fixed << std::setprecision(4) << asks_.begin()->first;
    }

    std::cout << "\n";
    
}

size_t Orderbook::get_max_levels() const {
    return MAX_LEVELS;
}

double Orderbook::get_exchange_bid_volume(const std::string& exchange_id, double price) const {
    auto user_it = exchange_bids_.find(exchange_id);
    if (user_it != exchange_bids_.end()) {
        auto price_it = user_it->second.find(price);
        if (price_it != user_it->second.end()) {
            return price_it->second;
        }
    }

    return 0.0;
}

double Orderbook::get_exchange_ask_volume(const std::string& exchange_id, double price) const {
    auto price_it = exchange_asks_.find(exchange_id);
    if (price_it != exchange_asks_.end()) {
        auto user_it = price_it->second.find(price);
        if (user_it != price_it->second.end()) {
            return user_it->second;
        }
    }
    return 0.0;
}

void Orderbook::initialize_exchange(const std::string& exchange_id) {
    exchange_bids_[exchange_id];
    exchange_asks_[exchange_id];
}

void Orderbook::add_client(int client_socket) {
    if (std::find(client_send_list_.begin(), client_send_list_.end(), client_socket) == client_send_list_.end()) {
        client_send_list_.push_back(client_socket);
    }
}

void Orderbook::remove_client(int client_socket) {
    auto it = std::find(client_send_list_.begin(), client_send_list_.end(), client_socket);

    if (it != client_send_list_.end()) {
        client_send_list_.erase(it);
    } 
    else {
        std::cout << "Number not found in the vector.\n";
    }
}