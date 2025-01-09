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

    /*
    auto exchange = exchange_list[0];

    auto result = exchange_map[exchange]->return_request("https://api.exchange.coinbase.com/currencies");

    std::cout << *result << "\n";
    */
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
    std::cout << "in thread waiting for client\n";

    char buf[1024];
    std::string received_data;

    while (true) {
        ssize_t bytes_received = recv(client_socket, buf, sizeof(buf) - 1, 0);

        if (bytes_received == -1) [[unlikely]] {
            std::cerr << "recv failed\n";
            break;
        }

        if (bytes_received == 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        buf[bytes_received] = '\0';

        received_data.append(buf);

        int code = process_request(received_data, client_socket);

        if (code == -1) {
            std::cout << "Closed connection\n";
            break;
        }
    }

    close(client_socket);
}



int CoreComponent::process_request(std::string request, int client_socket) {
    if (request == "exit") {
        return -1;
    }

    

    return 0;
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



std::pair<std::string, std::string> CoreComponent::find_best_bbo_exchange(const std::string &ticker) {
    std::string bid_exchange = "", ask_exchange = "";
    double bid_price = -1, ask_price = -1;

    for (const std::string &exchange : exchange_list) {
        Exchange* exchange_ptr = exchange_map[exchange];

        auto BBO = exchange_ptr->return_bbo(ticker);

        if (bid_price == -1) {
            bid_price = BBO->bid;
            bid_exchange = exchange_ptr->get_name();
        }
        else if (bid_price < BBO->bid) {
            bid_price = BBO->bid;
            bid_exchange = exchange_ptr->get_name();
        }

        if (ask_price == -1) {
            ask_price = BBO->ask;
            ask_exchange = exchange_ptr->get_name();
        }
        else if (ask_price > BBO->ask) {
            ask_price = BBO->ask;
            ask_exchange = exchange_ptr->get_name();
        }
    }


    return std::make_pair(bid_exchange, ask_exchange);
}