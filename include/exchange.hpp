#pragma once

#ifndef EXCHANGE_HPP
#define EXCHANGE_HPP

#include <string>

class Exchange {
public:
    virtual ~Exchange() = default;

    virtual std::optional<std::string> return_request(const std::string &url) = 0;

    virtual std::optional<BBO> return_bbo(const std::string &ticker) = 0;

    virtual std::optional<Latest_Trade> return_last_trade(const std::string &ticker) = 0;

    virtual std::optional<Orderbook_State> return_current_orderbook(const std::string &ticker, int max_levels) = 0;

    virtual std::string get_asset_name_conversion(const std::string &name)  = 0;

    virtual std::string get_name() = 0;
};

#endif // EXCHANGE_HPP