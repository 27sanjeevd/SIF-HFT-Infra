#pragma once

/*
    Crypto.com
*/

#ifndef CRYPTO_WS_HPP
#define CRYPTO_WS_HPP

#include "../websocket.hpp"

#include <unordered_map>
#include <optional>
#include <string>
#include <memory>

class Crypto_WS : public WebsocketConnection {
public:
    Crypto_WS(std::shared_ptr<Orderbook> new_book, std::string &ws_id, std::shared_ptr<std::mutex> mutex);

    void SubscribeToChannel(std::string &currency_name, std::string &channel_name) override;

    std::optional<std::string> GetCurrencyName(uint32_t currency_id);

private:
    void HandleMessage(const std::string_view& message) override;

    std::string GetHost() const override;
    std::string GetTarget() const override;

    const std::string host_ = "stream.crypto.com";
    const std::string target_ = "/exchange/v1/market";

    const double fee_percentage_ = 0.0015;

    std::unordered_map<uint32_t, std::string> id_to_currency_;
};

#endif // CRYPTO_WS_HPP