#pragma once

#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <iostream>
#include <iomanip>

#include "data.hpp"

class Orderbook {
private:
    using ExchangeOrderMap = std::unordered_map<std::string, std::unordered_map<double, double>>;

    static constexpr size_t MAX_LEVELS = 10000;
    static constexpr size_t TOP_LEVELS = 8;

    std::map<double, double, std::greater<double>> bids_;
    std::map<double, double> asks_;

    ExchangeOrderMap exchange_bids_;
    ExchangeOrderMap exchange_asks_;

    template <typename T>
    void rebalance(T& orders_map);

    template <typename T>
    void update_level(const std::string& exchange_id, double price, double new_volume,
                    T& orders_map, ExchangeOrderMap& exchanges);

    template <typename T>
    void delete_level(const std::string& exchange_id, double price,
                    T& orders_map, ExchangeOrderMap& exchanges);

    double get_total_volume_at_price(double price, 
                                const ExchangeOrderMap& exchanges) const;

public:
    Orderbook();

    void update_bid(const std::string &exchange_id, double price, double new_volume);
    void update_ask(const std::string &exchange_id, double price, double new_volume);

    void print_bbo(); 
    size_t get_max_levels() const;

    double get_exchange_bid_volume(const std::string& exchange_id, double price) const;
    double get_exchange_ask_volume(const std::string& exchange_id, double price) const;

    void initialize_exchange(const std::string& exchange_id);
};

#endif  // ORDERBOOK_HPP