#include "../include/coinbase.hpp"
#include "simdjson.h"

#include <iostream>

using namespace simdjson;

std::optional<std::string> Coinbase::return_request(const std::string &url) {

    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return std::nullopt;
    }

    std::string responseString;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Coinbase::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
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

    return responseString;
}



std::optional<BBO> Coinbase::return_bbo(const std::string &ticker) {
    std::string url = std::string("https://api.exchange.coinbase.com/products/") + ticker + std::string("/ticker");

    std::optional<std::string> response = return_request(url);
    if (!response) {
        return std::nullopt;
    }
    
    BBO output;

    simdjson::ondemand::parser parser;

    try {
        simdjson::ondemand::document doc = parser.iterate(*response);

        std::string_view ask = doc["ask"];
        std::string_view bid = doc["bid"];

        stringViewToDouble(bid, output.bid);
        stringViewToDouble(ask, output.ask);

        return output;
    }
    catch (...) {
        return std::nullopt;
    }
}



std::optional<Latest_Trade> Coinbase::return_last_trade(const std::string &ticker) {
    std::string url = std::string("https://api.exchange.coinbase.com/products/") + ticker + std::string("/ticker");

    std::optional<std::string> response = return_request(url);
    if (!response) {
        return std::nullopt;
    }
    
    Latest_Trade output;

    simdjson::ondemand::parser parser;

    try {
        simdjson::ondemand::document doc = parser.iterate(*response);

        std::string_view price = doc["price"];
        std::string_view size = doc["size"];

        stringViewToDouble(price, output.price);
        stringViewToDouble(size, output.size);

        return output;
    }
    catch (...) {
        return std::nullopt;
    }
}   



size_t Coinbase::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response) {
    size_t totalSize = size * nmemb;
    response->append((char *)contents, totalSize);
    return totalSize;
}

bool Coinbase::stringViewToDouble(const std::string_view& view, double& value) {
    if (view.empty()) {
        return false;
    }
    
    std::string temp(view);
    char* end;
    value = std::strtod(temp.c_str(), &end);
    return end == temp.c_str() + temp.length();
}