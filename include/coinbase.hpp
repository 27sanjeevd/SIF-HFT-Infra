#pragma once

#ifndef COINBASE_HPP
#define COINBASE_HPP

#include "data.hpp"
#include "../include/exchange.hpp"

#include <curl/curl.h>
#include <optional>
#include <string>
#include <unordered_map>

class Coinbase : public Exchange {
public:
    Coinbase();

    std::optional<std::string> return_request(const std::string &url) override;

    std::optional<BBO> return_bbo(const std::string &ticker) override;

    std::optional<Latest_Trade> return_last_trade(const std::string &ticker) override;

    std::optional<Orderbook_State> return_current_orderbook(const std::string &ticker, int max_levels) override;

    std::string get_asset_name_conversion(const std::string &name) override;

    std::string get_name() override;

private:
    static inline size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response);

    bool stringViewToDouble(const std::string_view& view, double& value);

    template <typename T>
    std::vector<std::tuple<double, double, int>> parse_levels(T input, int max_levels);


    std::unordered_map<std::string, std::string> assetToExchangeName;

    std::string name = "Coinbase";
};

#endif // COINBASE_HPP