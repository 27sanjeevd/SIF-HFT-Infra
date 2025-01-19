#pragma once

#ifndef DATA_HPP
#define DATA_HPP

#include <vector>
#include <tuple>

struct BBO {
    double bid;
    double ask;
};

struct Latest_Trade {
    double price;
    double size;
};

struct Price_Level {
    double price;
    double volume;
};

//price, volume, num orders
struct Orderbook_State {
    std::vector<std::tuple<double, double, uint64_t>> bids; 
    std::vector<std::tuple<double, double, uint64_t>> asks; 
};

struct OrderUpdate {
    std::string_view side;
    double price_level;
    double new_quantity;
};

#endif // DATA_HPP