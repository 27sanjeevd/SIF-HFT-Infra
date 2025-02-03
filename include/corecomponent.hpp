#pragma once

#ifndef CORECOMPONENT_HPP
#define CORECOMPONENT_HPP

#include "data.hpp"
#include "exchange.hpp"
#include "websockets/coinbase_ws.hpp"

#include <vector>
#include <unordered_map>
#include <memory>

class CoreComponent {
public:
    CoreComponent(std::unordered_map<uint32_t, std::string> &&id_map, 
        std::unordered_map<std::string, Exchange*> &&ptr_map, std::vector<std::string> &&list);

    void Run();

    void ReceiveConnections();

    void ConnectionHandler(int client_socket);

    int ProcessRequest(const char* request, int client_socket);

    void AddWebsocketConnection(int currency_id);

private:
    int server_fd_ = -1;

    std::unordered_map<uint32_t, std::shared_ptr<Orderbook>> open_orderbooks_;
    std::unordered_map<int, std::vector<uint32_t>> client_subscribe_list_;

    std::unordered_map<uint32_t, std::string> exchange_id_to_name_;
    std::unordered_map<std::string, Exchange*> exchange_map_;
    std::vector<std::string> exchange_list_;

    void ToNetworkOrder(double value, char* buffer);
};

#endif // CORECOMPONENT_HPP
