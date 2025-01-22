#pragma once

#ifndef CORECOMPONENT_HPP
#define CORECOMPONENT_HPP

#include "data.hpp"
#include "exchange.hpp"
#include "websockets/coinbase_ws.hpp"

#include <vector>
#include <unordered_map>

class CoreComponent {
public:
    CoreComponent(std::unordered_map<uint32_t, std::string> &&id_map, 
        std::unordered_map<std::string, Exchange*> &&ptr_map, std::vector<std::string> &&list);

    void Run();

    void ReceiveConnections();

    void ConnectionHandler(int client_socket);

    int ProcessRequest(const char* request, int client_socket);

    std::optional<Orderbook_State> GetTopNLevels(const std::string &ticker, int n);

    std::optional<BBO> GetBestBBO(const std::string &ticker);

private:
    int server_fd_ = -1;

    std::unordered_map<uint32_t, std::string> exchange_id_to_name_;
    std::unordered_map<std::string, Exchange*> exchange_map_;
    std::vector<std::string> exchange_list_;

    Orderbook curr_book_;

    void ToNetworkOrder(double value, char* buffer);

    void SendBestBBO(const char* request, int client_socket);

    void SendBestBook(const char* request, int client_socket);

    void SendLatestTrade(const char* request, int client_socket);
};

#endif // CORECOMPONENT_HPP
