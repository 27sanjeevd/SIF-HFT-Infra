#include "include/coinbase.hpp"

#include <iostream>
#include <type_traits>
#include <concepts>

template <typename T>
concept HasRequiredMethods = requires(T t, const std::string &url, const std::string &ticker, int max_levels) {
    { t.return_request(url) } -> std::same_as<std::optional<std::string>>;
    { t.return_bbo(ticker) } -> std::same_as<std::optional<BBO>>;
    { t.return_last_trade(ticker) } -> std::same_as<std::optional<Latest_Trade>>;
    { t.return_current_orderbook(ticker, max_levels) } -> std::same_as<std::optional<Orderbook_State>>;
};



int main() {
    // Static assert all the connectivity classes
    static_assert(HasRequiredMethods<Coinbase>, "Coinbase does not satisfy the required methods");

    Coinbase coinbase;
    
    std::optional<std::string> output = coinbase.return_request("https://api.exchange.coinbase.com/products/eth-usd/ticker");

    if (output) {
        std::cout << *output << "\n";
    }
    else {
        std::cout << "1------\n";
    }

    std::optional<BBO> best_bbo = coinbase.return_bbo("eth-usd");
    if (best_bbo) {
        std::cout << (*best_bbo).bid << " " << (*best_bbo).ask << "\n";
    }
    else {
        std::cout << "2------\n";
    }

    std::optional<Latest_Trade> latest_trade = coinbase.return_last_trade("eth-usd");
    if (latest_trade) {
        std::cout << (*latest_trade).price << " " << (*latest_trade).size << "\n";
    }
    else {
        std::cout << "2------\n";
    }

    std::optional<Orderbook_State> current_orderbook = coinbase.return_current_orderbook("eth-usd", 5);
    
    if (current_orderbook) {
        for (auto level : (*current_orderbook).bids) {
            std::cout << "Tuple values: ("
              << std::get<0>(level) << ", "
              << std::get<1>(level) << ", "
              << std::get<2>(level) << ")" << std::endl;
        }
        std::cout << "----------------\n";

        for (auto level : (*current_orderbook).asks) {
            std::cout << "Tuple values: ("
              << std::get<0>(level) << ", "
              << std::get<1>(level) << ", "
              << std::get<2>(level) << ")" << std::endl;
        }
    }
    else {
        std::cout << "3----\n";
    }
    

    return 0;
}