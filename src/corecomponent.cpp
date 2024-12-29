#include "../include/corecomponent.hpp"

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <map>

#define PORT 8080

CoreComponent::CoreComponent(std::unordered_map<std::string, Exchange*> &&map,
    std::vector<std::string> &&list) : exchange_map(std::move(map)), exchange_list(std::move(list)) {}



void CoreComponent::run() {
    std::thread listeningThread(&CoreComponent::receive_connections, this);

    listeningThread.join();
}



void CoreComponent::receive_connections() {

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return;
    }

    std::vector<std::thread> threads;

    while (true) {
        std::cout << "waiting for connections\n";
        sockaddr_in client_address{};
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_fd, (struct sockaddr*) &client_address, &client_address_len);
        
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        threads.emplace_back([this, client_socket]() {
            this->connection_handler(client_socket);
        });    
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    close(server_fd);
}



void CoreComponent::connection_handler(int client_socket) {
    std::cout << "HI\n";
}



Orderbook_State CoreComponent::get_top_n_levels(const std::string &ticker, int n) {
    Orderbook_State output;

    std::map<double, std::pair<double, int>> bids;
    std::map<double, std::pair<double, int>> asks;

    for (const std::string &exchange : exchange_list) {
        Exchange* exchange_ptr = exchange_map[exchange];

        auto orderbook_state = exchange_ptr->return_current_orderbook(ticker, n);

        if (orderbook_state) {
            // dont need to duplicate code for bids vs asks

            auto exchange_bids = (*orderbook_state).bids;

            for (auto bid_tuple : exchange_bids) {
                double key = std::get<0>(bid_tuple);

                bids[key].first += std::get<1>(bid_tuple);
                bids[key].second += std::get<2>(bid_tuple);
            }

            auto exchange_asks = (*orderbook_state).asks;

            for (auto ask_tuple : exchange_asks) {
                double key = std::get<0>(ask_tuple);

                asks[key].first += std::get<1>(ask_tuple);
                asks[key].second += std::get<2>(ask_tuple);
            }
        }
    }

    if (bids.size() > 0) {
        int count = 0;
        for (auto it = bids.rbegin(); it != bids.rend() && count < n; it++, count++) {
            double key = it->first;
            auto value_pair = it->second;

            output.bids.emplace_back(key, value_pair.first, value_pair.second);
        }

        count = 0;
        for (auto it = asks.begin(); it != asks.end() && count < n; it++, count++) {
            double key = it->first;
            auto value_pair = it->second;

            output.asks.emplace_back(key, value_pair.first, value_pair.second);
        }
    }

    return output;
}