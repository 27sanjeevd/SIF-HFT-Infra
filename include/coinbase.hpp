#pragma once

#ifndef COINBASE_HPP
#define COINBASE_HPP

#include "data.hpp"

#include <curl/curl.h>
#include <optional>
#include <string>

class Coinbase {
    public:
        std::optional<std::string> return_request(const std::string &url);

        std::optional<BBO> return_bbo(const std::string &ticker);

        std::optional<Latest_Trade> return_last_trade(const std::string &ticker);

        std::optional<Orderbook_State> return_current_orderbook(const std::string &ticker, int max_levels);

    private:
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response);

        bool stringViewToDouble(const std::string_view& view, double& value);

        template <typename T>
        std::vector<std::tuple<double, double, int>> parse_levels(T input, int max_levels);
};

#endif // COINBASE_HPP