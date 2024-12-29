#include "../include/corecomponent.hpp"

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

#define PORT 8080

CoreComponent::CoreComponent(std::unordered_map<std::string, Exchange*> &&map) : exchange_map(std::move(map)) {}

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