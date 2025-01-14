#pragma once

#ifndef EXCHANGE_HPP
#define EXCHANGE_HPP

#include <string>

class Exchange {
public:
    virtual ~Exchange() = default;

    virtual std::optional<std::string> ReturnRequest(const std::string &url) = 0;

    virtual std::optional<BBO> ReturnBBO(const std::string &ticker) = 0;

    virtual std::optional<Latest_Trade> ReturnLastTrade(const std::string &ticker) = 0;

    virtual std::optional<Orderbook_State> ReturnCurrentOrderbook(const std::string &ticker, int max_levels) = 0;

    virtual std::optional<std::string> get_asset_name_conversion(const std::string &name)  = 0;

    virtual std::string get_name() = 0;
};

#endif // EXCHANGE_HPP