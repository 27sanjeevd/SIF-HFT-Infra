#include "../include/orderbook.hpp"

Orderbook::Orderbook() {
    top_bids_.reserve(TOP_LEVELS);
    top_asks_.reserve(TOP_LEVELS);
}

template <typename T>
void Orderbook::rebalance(std::vector<Price_Level>& top, T& remaining, std::unordered_set<double> price_check) {

    while (top.size() < TOP_LEVELS && !remaining.empty()) {
        auto it = remaining.begin();
        top.push_back({it->first, it->second});
        remaining.erase(it);
    }

    while (top.size() > TOP_LEVELS) {
        auto& last = top.back();
        remaining[last.price] = last.volume;
        top.pop_back();
    }

    while (top.size() + remaining.size() > MAX_LEVELS) {
        auto it = std::prev(remaining.end());
        remaining.erase(it);
        
        price_check.erase(it->first);
    }
}

template <typename T>
void Orderbook::add_level(double price, double volume, std::vector<Price_Level>& top, T& remaining,
                         bool is_bid, std::unordered_set<double> price_check) {
    
    auto compare = [is_bid] (const Price_Level& level, double p) {
        return is_bid ? level.price > p : level.price < p;
    };

    auto it = std::lower_bound(top.begin(), top.end(), price, compare);

    if (it != top.end() || top.size() < TOP_LEVELS) {
        top.insert(it, {price, volume});
    } else {
        remaining[price] = volume;
    }

    remaining[price] = volume;

    rebalance(top, remaining, price_check);
}

template <typename T>
void Orderbook::update_level(double price, double new_volume, std::vector<Price_Level>& top, 
                            T& remaining, bool is_bid) {

    auto it = std::find_if(top.begin(), top.end(),
        [price](const Price_Level& level) { return level.price == price; });
    
    if (it != top.end()) {
        it->volume = new_volume;
        return;
    }
    

    remaining[price] = new_volume;
}

template <typename T>
void Orderbook::delete_level(double price, std::vector<Price_Level>& top, T& remaining) {
    
    
    auto it = std::find_if(top.begin(), top.end(),
        [price](const Price_Level& level) { return level.price == price; });
    
    if (it != top.end()) {
        top.erase(it);
    }
    
    remaining.erase(price);
    
    //rebalance(top, remaining);
}

void Orderbook::add_bid(double price, double volume) {
    add_level(price, volume, top_bids_, remaining_bids_, true, bids_);
}

void Orderbook::add_ask(double price, double volume) {
    add_level(price, volume, top_asks_, remaining_asks_, false, asks_);
}

void Orderbook::update_bid(double price, double new_volume) {

    if (bids_.count(price) == 0) {
        if (top_bids_.size() + remaining_bids_.size() == MAX_LEVELS && price < worst_bid_) {
            return;
        }
        bids_.insert(price);
        worst_bid_ = std::min(worst_bid_, price);

        add_level(price, new_volume, top_bids_, remaining_bids_, true, bids_);
    }
    else {
        if (new_volume == 0) {
            delete_level(price, top_bids_, remaining_bids_);
        }
        else {
            update_level(price, new_volume, top_bids_, remaining_bids_, true);
        }
    }
}

void Orderbook::update_ask(double price, double new_volume) {

    if (asks_.count(price) == 0) {
        if (top_asks_.size() + remaining_asks_.size() == MAX_LEVELS && price > worst_ask_) {
            return;
        }
        asks_.insert(price);
        worst_ask_ = std::max(worst_ask_, price);

        add_level(price, new_volume, top_asks_, remaining_asks_, false, asks_);
    }
    else {
        if (new_volume == 0) {
            delete_level(price, top_asks_, remaining_asks_);
        }
        else {
            update_level(price, new_volume, top_asks_, remaining_asks_, false);
        }
    }
}

void Orderbook::delete_bid(double price) {
    delete_level(price, top_bids_, remaining_bids_);
}

void Orderbook::delete_ask(double price) {
    delete_level(price, top_asks_, remaining_asks_);
}