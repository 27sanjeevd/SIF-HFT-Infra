#include "../include/corecomponent.hpp"
#include "../include/websocket.hpp"

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <libkern/OSByteOrder.h>
#include <iomanip>
#include <unistd.h>
#include <iostream>
#include <map>
#include <mutex>
#include <algorithm>

static constexpr int kPort = 8080;

CoreComponent::CoreComponent(std::unordered_map<uint32_t, std::string> &&id_map, 
    std::unordered_map<std::string, Exchange*> &&ptr_map, std::vector<std::string> &&list) : 
        exchange_id_to_name_(std::move(id_map)), exchange_map_(std::move(ptr_map)), exchange_list_(std::move(list)) {}



void CoreComponent::Run() {
    std::thread listeningThread(&CoreComponent::ReceiveConnections, this);

    listeningThread.join();
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


#pragma pack(1)
struct ReceivedData {
    uint32_t message_type;
    uint32_t currency_name;
    uint32_t num_levels;
};
#pragma pack()


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

        uint32_t return_code = ProcessRequest(socket_buffer, client_socket);

        if (return_code == 0) {
            std::cout << "Closed connection\n";
            break;
        }
    }

    close(client_socket);
}



int CoreComponent::ProcessRequest(const char* request, int client_socket) {
    ReceivedData data;

    std::memcpy(&data.message_type, request, sizeof(uint32_t));
    data.message_type = OSSwapHostToBigInt32(data.message_type);

    if (data.message_type == 0) {
        std::cout << "exit\n";
        for (auto currency : client_subscribe_list_[client_socket]) {
            open_orderbooks_[currency]->remove_client(client_socket);
        }
    }
    else if (data.message_type == 1) {
        std::memcpy(&data.currency_name, request + 4, sizeof(uint32_t));
        data.currency_name = OSSwapHostToBigInt32(data.currency_name);

        std::memcpy(&data.num_levels, request + 8, sizeof(uint32_t));
        data.num_levels = OSSwapHostToBigInt32(data.num_levels);
        
        if (!open_orderbooks_.contains(data.currency_name)) {
            AddWebsocketConnection(data.currency_name);
        }
        
        open_orderbooks_[data.currency_name]->add_client(client_socket);
        client_subscribe_list_[client_socket].push_back(data.currency_name);
    }
    else if (data.message_type == 2) {
        std::memcpy(&data.currency_name, request + 4, sizeof(uint32_t));
        data.currency_name = OSSwapHostToBigInt32(data.currency_name);

        for (auto currency : client_subscribe_list_[client_socket]) {
            if (currency == data.currency_name) {
                open_orderbooks_[currency]->remove_client(client_socket);
            }
        }
    }

    return data.message_type;
}

void CoreComponent::AddWebsocketConnection(uint32_t currency_id) {

    auto new_orderbook = std::make_shared<Orderbook>(currency_id);
    std::shared_ptr<std::mutex> mtx = std::make_shared<std::mutex>();
    open_orderbooks_[currency_id] = new_orderbook;
    
    auto coinbase_thread_func = [this, currency_id, new_orderbook, mtx]() {
        std::string new_id = "coinbase";

        Coinbase_WS ws(new_orderbook, new_id, mtx);

        auto currency_name = ws.GetCurrencyName(currency_id);
        if (!currency_name) {
            return;
        }

        std::string currency = *currency_name;
        std::string channel = "level2";

        ws.Connect(currency, channel);
    };

    std::thread coinbase_thread(coinbase_thread_func);
    coinbase_thread.detach();
    
    
    auto crypto_thread_func = [this, currency_id, new_orderbook, mtx]() {
        std::string new_id = "crypto";

        Crypto_WS ws(new_orderbook, new_id, mtx);

        auto currency_name = ws.GetCurrencyName(currency_id);
        if (!currency_name) {
            return;
        }

        std::string currency = *currency_name;
        std::string channel = "SNAPSHOT_AND_UPDATE";

        ws.Connect(currency, channel);
    };

    std::thread crypto_thread(crypto_thread_func);
    crypto_thread.detach();
}


void CoreComponent::ToNetworkOrder(double value, char* buffer) {
    uint64_t raw;
    std::memcpy(&raw, &value, sizeof(raw));
    raw = OSSwapHostToBigInt64(raw);
    std::memcpy(buffer, &raw, sizeof(raw));
}
