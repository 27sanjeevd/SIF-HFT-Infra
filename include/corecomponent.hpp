#pragma once

#ifndef CORECOMPONENT_HPP
#define CORECOMPONENT_HPP

#include "data.hpp"
#include "../include/exchange.hpp"

#include <vector>
#include <unordered_map>

class CoreComponent {
public:
    CoreComponent(std::unordered_map<std::string, Exchange*> &&map, std::vector<std::string> &&list);

    void run();

    void receive_connections();

    void connection_handler(int client_socket);

    int process_request(const char* request, int client_socket);

    Orderbook_State get_top_n_levels(const std::string &ticker, int n);

    std::pair<std::string, std::string> find_best_bbo_exchange(const std::string &ticker);

    BBO get_best_bbo(const std::string &ticker);

private:
    int server_fd = -1;

    std::unordered_map<std::string, Exchange*> exchange_map;
    std::vector<std::string> exchange_list;

    void to_network_order(double value, char* buffer);

    void send_best_bbo(const char* request, int client_socket);
};

#endif // CORECOMPONENT_HPP
