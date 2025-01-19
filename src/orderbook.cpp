#include "../include/orderbook.hpp"

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
    /*

    if (new_volume == 0) {
        delete_level(exchange_id, price, orders_map, exchanges);
    }
    else {
        orders_map[price] = new_volume;
    }
    */

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
    

    //orders_map.erase(price);
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



void Orderbook::update_bid(const std::string &exchange_id, double price, double new_volume) {
    update_level(exchange_id, price, new_volume, bids_, exchange_bids_);
    print_bbo();
}

void Orderbook::update_ask(const std::string &exchange_id, double price, double new_volume) {
    update_level(exchange_id, price, new_volume, asks_, exchange_asks_);
    print_bbo();
}

void Orderbook::print_bbo() {

    if (bids_.size() > 0) {
        std::cout << "Bid: " << std::fixed << std::setprecision(2) << bids_.begin()->first << " ";
    }
    if (asks_.size() > 0) {
        std::cout << "Ask: " << std::fixed << std::setprecision(2) << asks_.begin()->first;
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