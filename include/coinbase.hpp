#pragma once

#ifndef COINBASE_HPP
#define COINBASE_HPP

#include "data.hpp"
#include "exchange.hpp"

#include <curl/curl.h>
#include <optional>
#include <string>
#include <unordered_map>

class Coinbase : public Exchange {
public:
    Coinbase();

    std::optional<std::string> ReturnRequest(const std::string &url) override;

    std::optional<BBO> ReturnBBO(const std::string &ticker) override;

    std::optional<Latest_Trade> ReturnLastTrade(const std::string &ticker) override;

    std::optional<Orderbook_State> ReturnCurrentOrderbook(const std::string &ticker, int max_levels) override;

    std::optional<std::string> get_asset_name_conversion(const std::string &name) override;

    std::string get_name() override;

private:
    static inline size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response);

    bool StringViewToDouble(const std::string_view& view, double& value);

    template <typename T>
    std::vector<std::tuple<double, double, uint64_t>> ParseLevels(T input, int max_levels);


    std::unordered_map<std::string, std::string> asset_to_exchange_name_;
    std::string name_ = "Coinbase";
};

#endif // COINBASE_HPP