#include "include/coinbase.hpp"
#include "include/corecomponent.hpp"

#include <iostream>
#include <type_traits>
#include <concepts>
#include <unordered_map>

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

    std::unordered_map<std::string, Exchange*> exchange_map;
    exchange_map[coinbase.get_name()] = &coinbase;

    CoreComponent cc(std::move(exchange_map));
    cc.run();
    

    return 0;
}