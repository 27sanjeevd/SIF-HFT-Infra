#pragma once

#ifndef CORECOMPONENT_HPP
#define CORECOMPONENT_HPP

#include "../include/exchange.hpp"

#include <vector>
#include <unordered_map>

class CoreComponent {
public:
    CoreComponent(std::unordered_map<std::string, Exchange*> &&map);

    void run();

    void receive_connections();

    void connection_handler(int client_socket);

private:
    int server_fd = -1;

    std::unordered_map<std::string, Exchange*> exchange_map;
};

#endif // CORECOMPONENT_HPP
