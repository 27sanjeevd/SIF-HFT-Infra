#pragma once

#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/json.hpp>
#include <simdjson.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "orderbook.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class WebsocketConnection {
public:
    WebsocketConnection(Orderbook &new_book, std::string &ws_id);

    void Initialize();

    void Connect(std::string &currency_name, std::string &channel_name);

    void EstablishConnection();

    virtual void SubscribeToChannel(std::string &currency_name, std::string &channel_name) = 0;

    void StartMessageLoop();

    void ProcessMessage(const beast::flat_buffer& buffer);

    void Disconnect();

protected:
    virtual void HandleMessage(const std::string& message) = 0;

    virtual std::string GetHost() const = 0;

    net::io_context ioc_;
    ssl::context ctx_{ssl::context::tlsv12_client};
    websocket::stream<beast::ssl_stream<tcp::socket>> ws_{ioc_, ctx_};
    simdjson::ondemand::parser json_parser_;
    Orderbook& curr_book_;

    std::string id_;
};

#endif // WEBSOCKET_HPP