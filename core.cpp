#include "include/coinbase.hpp"

#include <iostream>
#include <type_traits>
#include <concepts>

template <typename T>
concept HasRequiredMethods = requires(T t, const std::string& url) {
    { t.return_request(url) } -> std::same_as<std::optional<std::string>>;
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

    return 0;
}