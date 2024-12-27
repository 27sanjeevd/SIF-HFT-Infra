#pragma once

#ifndef COINBASE_HPP
#define COINBASE_HPP

#include <curl/curl.h>
#include <optional>
#include <string>

class Coinbase {
  public:
    std::optional<std::string> return_request(const std::string &url);

  private:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response);
};

#endif // COINBASE_HPP