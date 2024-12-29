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

    Orderbook_State get_top_n_levels(const std::string &ticker, int n);

private:
    int server_fd = -1;

    std::unordered_map<std::string, Exchange*> exchange_map;
    std::vector<std::string> exchange_list;
};

#endif // CORECOMPONENT_HPP
