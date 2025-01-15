#pragma once

#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <map>
#include <algorithm>

#include "../../include/data.hpp"

class Orderbook {
private:
    static constexpr size_t MAX_LEVELS = 100;
    static constexpr size_t TOP_LEVELS = 8;

    std::vector<Price_Level> top_bids_;
    std::vector<Price_Level> top_asks_;
    std::map<double, double, std::greater<double>> remaining_bids_;
    std::map<double, double> remaining_asks_;

    template <typename T>
    void rebalance(std::vector<Price_Level>& top, T& remaining);

    template <typename T>
    void add_level(double price, double volume, std::vector<Price_Level>& top, T& remaining, 
                  bool is_bid);

    template <typename T>
    void update_level(double price, double new_volume, std::vector<Price_Level>& top, T& remaining,
                bool is_bid);

    template <typename T>
    void delete_level(double price, std::vector<Price_Level>& top, T& remaining);

public:
    void add_bid(double price, double volume);

    void add_ask(double price, double volume);

    void update_bid(double price, double new_volume);

    void update_ask(double price, double new_volume);

    void delete_bid(double price);

    void delete_ask(double price);
    
    const std::vector<Price_Level>& get_top_bids() const;

    const std::vector<Price_Level>& get_top_asks() const;
};

#endif  // ORDERBOOK_HPP