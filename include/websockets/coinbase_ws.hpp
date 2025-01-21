#pragma once

#ifndef COINBASE_WS_HPP
#define COINBASE_WS_HPP

#include "../websocket.hpp"

class Coinbase_WS : public WebsocketConnection {
public:
    Coinbase_WS(Orderbook &new_book, std::string &ws_id);

    void SubscribeToChannel(std::string &currency_name, std::string &channel_name) override;

private:
    void HandleMessage(const std::string& message) override;

    std::string GetHost() const override;

    const std::string host_ = "advanced-trade-ws.coinbase.com";
};

#endif // COINBASE_WS_HPP