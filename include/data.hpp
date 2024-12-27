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

struct Orderbook_State {
    std::vector<std::tuple<double, double, int>> bids; 
    std::vector<std::tuple<double, double, int>> asks; 
};

#endif // DATA_HPP