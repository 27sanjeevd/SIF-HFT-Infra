#include "../include/coinbase.hpp"
#include "simdjson.h"

#include <iostream>
#include <regex>

using namespace simdjson;

Coinbase::Coinbase() {
    asset_to_exchange_name_["btc"] = "btc-usd";
    asset_to_exchange_name_["eth"] = "eth-usd";
    asset_to_exchange_name_["xrp"] = "xrp-usd";
    asset_to_exchange_name_["sol"] = "sol-usd";
    asset_to_exchange_name_["doge"] = "doge-usd";
    asset_to_exchange_name_["ada"] = "ada-usd";
}

std::optional<std::string> Coinbase::ReturnRequest(const std::string &url) {

    CURL *curl = curl_easy_init();
    if (!curl) [[unlikely]] {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return std::nullopt;
    }

    std::string response_string;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Coinbase::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return std::nullopt;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response_string;
}



std::optional<BBO> Coinbase::ReturnBBO(const std::string &ticker) {
    std::string url = std::string("https://api.exchange.coinbase.com/products/") + ticker + std::string("/ticker");

    std::optional<std::string> response = ReturnRequest(url);
    if (!response) [[unlikely]] {
        return std::nullopt;
    }
    
    BBO output;

    simdjson::ondemand::parser parser;

    try {
        simdjson::ondemand::document doc = parser.iterate(*response);

        std::string_view ask = doc["ask"];
        std::string_view bid = doc["bid"];

        StringViewToDouble(bid, output.bid);
        StringViewToDouble(ask, output.ask);

        return output;
    }
    catch (...) {
        return std::nullopt;
    }
}



std::optional<Latest_Trade> Coinbase::ReturnLastTrade(const std::string &ticker) {
    std::string url = std::string("https://api.exchange.coinbase.com/products/") + ticker + std::string("/ticker");

    std::optional<std::string> response = ReturnRequest(url);
    if (!response) [[unlikely]] {
        return std::nullopt;
    }
    
    Latest_Trade output;

    simdjson::ondemand::parser parser;

    try {
        simdjson::ondemand::document doc = parser.iterate(*response);

        std::string_view price = doc["price"];
        std::string_view size = doc["size"];

        StringViewToDouble(price, output.price);
        StringViewToDouble(size, output.size);

        return output;
    }
    catch (...) {
        return std::nullopt;
    }
}



std::optional<Orderbook_State> Coinbase::ReturnCurrentOrderbook(const std::string &ticker, int max_levels) {
    std::string url = std::string("https://api.exchange.coinbase.com/products/") + ticker + std::string("/book?level=2");

    std::optional<std::string> response = ReturnRequest(url);
    if (!response) [[unlikely]] {
        return std::nullopt;
    }

    Orderbook_State output;

    simdjson::ondemand::parser parser;

    try {
        simdjson::ondemand::document doc = parser.iterate(*response);

        std::vector<std::tuple<double, double, uint64_t>> bids = ParseLevels(doc["bids"], max_levels);
        std::vector<std::tuple<double, double, uint64_t>> asks = ParseLevels(doc["asks"], max_levels);

        output.bids = bids;
        output.asks = asks;

        return output;
    }
    catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> Coinbase::get_asset_name_conversion(const std::string &name) {
    if (asset_to_exchange_name_.count(name) != 0) {
        return asset_to_exchange_name_[name];
    }

    return std::nullopt;
}

std::string Coinbase::get_name() {
    return name_;
}



inline size_t Coinbase::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response) {
    size_t total_size = size * nmemb;
    response->append((char *)contents, total_size);
    return total_size;
}



bool Coinbase::StringViewToDouble(const std::string_view& view, double& value) {
    if (view.empty()) [[unlikely]] {
        return false;
    }
    
    std::string temp(view);
    char* end;
    value = std::strtod(temp.c_str(), &end);
    return end == temp.c_str() + temp.length();
}



// Messy, can be cleaned up
template <typename T>
std::vector<std::tuple<double, double, uint64_t>> Coinbase::ParseLevels(T input, int max_levels) {
    std::vector<std::tuple<double, double, uint64_t>> output;

    auto a = input.get_array();
    auto& arr = a.value();

    int count = 0;

    for (auto elem : arr) {
        if (count >= max_levels) {
            break;
        }
        double first = 0.0, second = 0.0;
        uint64_t third = 0;

        int index = 0;
        for (auto val : elem) {
            if (index == 0) {
                std::string_view sv = val.get_string().value_unsafe();
                std::string temp(sv);
                first = std::stod(temp);
            }
            else if (index == 1) {
                std::string_view sv = val.get_string().value_unsafe();
                std::string temp(sv);
                second = std::stod(temp);
            }
            else if (index == 2) {
                third = val.get_int64();
            }
            index++;    
        }

        output.emplace_back(first, second, third);
        count++;
    }

    return output;
}