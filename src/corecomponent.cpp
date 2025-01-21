#include "../include/corecomponent.hpp"
#include "../include/websocket.hpp"
#include "../include/websockets/coinbase_ws.hpp"

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <libkern/OSByteOrder.h>
#include <iomanip>
#include <unistd.h>
#include <iostream>
#include <map>
#include <algorithm>

static constexpr int kPort = 8080;

CoreComponent::CoreComponent(std::unordered_map<uint32_t, std::string> &&id_map, 
    std::unordered_map<std::string, Exchange*> &&ptr_map, std::vector<std::string> &&list) : 
        exchange_id_to_name_(std::move(id_map)), exchange_map_(std::move(ptr_map)), exchange_list_(std::move(list)) {}



void CoreComponent::Run() {
    //std::thread listeningThread(&CoreComponent::ReceiveConnections, this);

    //listeningThread.join();

    
    Orderbook new_book;
    std::string new_id = "id";

    Coinbase_WS new_ws(new_book, new_id);

    std::string currency = "BTC-USD";
    std::string channel = "level2";

    new_ws.Connect(currency, channel);
}



void CoreComponent::ReceiveConnections() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        perror("Socket creation failed");
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(kPort);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd_);
        return;
    }

    if (listen(server_fd_, 3) < 0) {
        perror("Listen failed");
        close(server_fd_);
        return;
    }

    std::vector<std::thread> threads_list;

    while (true) {
        std::cout << "waiting for connections\n";
        sockaddr_in client_address{};
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_fd_, (struct sockaddr*) &client_address, &client_address_len);
        
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        threads_list.emplace_back([this, client_socket]() {
            this->ConnectionHandler(client_socket);
        });    
    }

    for (auto& thread : threads_list) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    close(server_fd_);
}



void CoreComponent::ConnectionHandler(int client_socket) {
    std::cout << "in thread waiting for client\n";

    char socket_buffer[1024];

    while (true) {
        ssize_t bytes_received = recv(client_socket, socket_buffer, sizeof(socket_buffer) - 1, 0);

        if (bytes_received == -1) [[unlikely]] {
            std::cerr << "recv failed\n";
            break;
        }

        if (bytes_received == 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        socket_buffer[bytes_received] = '\0';

        int return_code = ProcessRequest(socket_buffer, client_socket);

        if (return_code == -1) {
            std::cout << "Closed connection\n";
            break;
        }
    }

    close(client_socket);
}



int CoreComponent::ProcessRequest(const char* request, int client_socket) {
    char descriptor[17] = {0};
    std::memcpy(descriptor, request + 4, 16);
    std::string message_descriptor = std::string(descriptor);


    if (message_descriptor == "exit") {
        return -1;
    }
    else if (message_descriptor == "best_bbo") {
        CoreComponent::SendBestBBO(request, client_socket);
    }
    else if (message_descriptor == "best_book") {
        CoreComponent::SendBestBook(request, client_socket);
    }
    else if (message_descriptor == "last_trade") {
        CoreComponent::SendLatestTrade(request, client_socket);
    }

    return 0;
}



std::optional<Orderbook_State> CoreComponent::GetTopNLevels(const std::string &ticker, int n) {
    Orderbook_State output;

    std::map<double, std::pair<double, uint64_t>> bids_map;
    std::map<double, std::pair<double, uint64_t>> asks_map;

    for (const std::string &exchange : exchange_list_) {
        Exchange* exchange_ptr = exchange_map_[exchange];

        auto name_conversion_opt = exchange_ptr->get_asset_name_conversion(ticker);
        if (!name_conversion_opt.has_value()) [[unlikely]] {
            return std::nullopt;
        }

        auto orderbook_state = exchange_ptr->ReturnCurrentOrderbook(name_conversion_opt.value(), n);
        if (!orderbook_state.has_value()) [[unlikely]] {
            return std::nullopt;
        }

        if (orderbook_state) {
            // dont need to duplicate code for bids vs asks

            auto exchange_bids = (*orderbook_state).bids;

            for (auto bid_tuple : exchange_bids) {
                double bid_price = std::get<0>(bid_tuple);

                bids_map[bid_price].first += std::get<1>(bid_tuple);
                bids_map[bid_price].second += std::get<2>(bid_tuple);
            }

            auto exchange_asks = (*orderbook_state).asks;

            for (auto ask_tuple : exchange_asks) {
                double ask_price = std::get<0>(ask_tuple);

                asks_map[ask_price].first += std::get<1>(ask_tuple);
                asks_map[ask_price].second += std::get<2>(ask_tuple);
            }
        }
    }

    if (bids_map.size() > 0) {
        int level_count = 0;
        for (auto bid_map_iterator = bids_map.rbegin(); bid_map_iterator != bids_map.rend() && level_count < n; bid_map_iterator++, level_count++) {
            double bid_price = bid_map_iterator->first;
            auto value = bid_map_iterator->second;

            output.bids.emplace_back(bid_price, value.first, value.second);
        }

        level_count = 0;
        for (auto ask_map_iterator = asks_map.begin(); ask_map_iterator != asks_map.end() && level_count < n; ask_map_iterator++, level_count++) {
            double bid_price = ask_map_iterator->first;
            auto value = ask_map_iterator->second;

            output.asks.emplace_back(bid_price, value.first, value.second);
        }
    }

    return output;
}

// add error condition if the ticker isn't present in the get_asset_name_conversion map
std::optional<BBO> CoreComponent::GetBestBBO(const std::string &ticker) {
    double best_bid = std::numeric_limits<double>::lowest(), best_ask = std::numeric_limits<double>::max();

    for (const auto &exchange : exchange_list_) {
        Exchange* exchange_ptr = exchange_map_[exchange];

        auto name_conversion_opt = exchange_ptr->get_asset_name_conversion(ticker);
        if (!name_conversion_opt.has_value()) [[unlikely]] {
            return std::nullopt;
        }

        auto BBO = exchange_ptr->ReturnBBO(name_conversion_opt.value());
        if (!BBO.has_value()) [[unlikely]] {
            return std::nullopt;
        }

        best_bid = std::max(best_bid, BBO->bid);
        best_ask = std::min(best_ask, BBO->ask);
    }

    BBO output = { .bid = best_bid, .ask = best_ask };
    return output;
}

void CoreComponent::ToNetworkOrder(double value, char* buffer) {
    uint64_t raw;
    std::memcpy(&raw, &value, sizeof(raw));
    raw = OSSwapHostToBigInt64(raw);
    std::memcpy(buffer, &raw, sizeof(raw));
}

void CoreComponent::SendBestBBO(const char* request, int client_socket) {
    char asset[5] = {0};
    std::memcpy(asset, request + 20, 4);
    std::string asset_name = std::string(asset);

    auto best_bbo_opt = CoreComponent::GetBestBBO(asset_name);
    if (!best_bbo_opt.has_value()) [[unlikely]] {
        uint32_t status = 0;
        char message[4];

        uint32_t network_status = OSSwapHostToBigInt32(status);
        std::memcpy(message, &network_status, 4);

        send(client_socket, message, 4, 0);
        return;
    }

    BBO best_bbo = best_bbo_opt.value();

    char message[24];
    uint32_t message_size = 16;

    uint32_t status = 1;
    uint32_t network_status = OSSwapHostToBigInt32(status);
    std::memcpy(message, &network_status, 4);

    uint32_t network_size = OSSwapHostToBigInt32(message_size);
    std::memcpy(message + 4, &network_size, 4);

    ToNetworkOrder(best_bbo.bid, message + 8);

    ToNetworkOrder(best_bbo.ask, message + 16);

    send(client_socket, message, sizeof(message), 0);
}



void CoreComponent::SendBestBook(const char* request, int client_socket) {
    char asset[5] = {0};
    std::memcpy(asset, request + 20, 4);
    std::string asset_name = std::string(asset);

    char num_levels[5] = {0};
    std::memcpy(num_levels, request + 24, 4);
    uint32_t level_count = 0;

    std::memcpy(&level_count, num_levels, sizeof(level_count));
    level_count = OSSwapBigToHostInt32(level_count);

    auto top_n_levels_opt = CoreComponent::GetTopNLevels(asset_name, level_count);
    if (!top_n_levels_opt.has_value()) [[unlikely]] {
        uint32_t status = 0;
        char message[4];

        uint32_t network_status = OSSwapHostToBigInt32(status);
        std::memcpy(message, &network_status, 4);

        send(client_socket, message, 4, 0);
        return;
    }
    
    Orderbook_State top_n_levels = top_n_levels_opt.value();

    uint64_t space_used = 0;

    char message[12 + level_count * 48];
    uint32_t message_size = 4 + level_count * 48;

    uint32_t status = 1;
    uint32_t network_status = OSSwapHostToBigInt32(status);
    std::memcpy(message + space_used, &network_status, 4);
    space_used += 4;


    uint32_t network_size = OSSwapHostToBigInt32(message_size);
    std::memcpy(message + space_used, &network_size, 4);
    space_used += 4;

    uint32_t network_levels = OSSwapHostToBigInt32(level_count);
    std::memcpy(message + space_used, &network_levels, 4);
    space_used += 4;


    for (auto x = 0; x < level_count; x++) {
        ToNetworkOrder(std::get<0>(top_n_levels.bids[x]), message + space_used);
        space_used += 8;
        ToNetworkOrder(std::get<1>(top_n_levels.bids[x]), message + space_used);
        space_used += 8;
        ToNetworkOrder(std::get<2>(top_n_levels.bids[x]), message + space_used);
        space_used += 8;

        ToNetworkOrder(std::get<0>(top_n_levels.asks[x]), message + space_used);
        space_used += 8;
        ToNetworkOrder(std::get<1>(top_n_levels.asks[x]), message + space_used);
        space_used += 8;
        ToNetworkOrder(std::get<2>(top_n_levels.asks[x]), message + space_used);
        space_used += 8;
    }

    send(client_socket, message, sizeof(message), 0);
}

void CoreComponent::SendLatestTrade(const char* request, int client_socket) {
    
}