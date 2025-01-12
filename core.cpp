#include "include/coinbase.hpp"
#include "include/corecomponent.hpp"

#include <iostream>
#include <type_traits>
#include <concepts>
#include <unordered_map>

template <typename T>
concept HasRequiredMethods = requires(T t, const std::string &url, const std::string &ticker, int max_levels, const std::string &asset_name) {
    { t.return_request(url) } -> std::same_as<std::optional<std::string>>;
    { t.return_bbo(ticker) } -> std::same_as<std::optional<BBO>>;
    { t.return_last_trade(ticker) } -> std::same_as<std::optional<Latest_Trade>>;
    { t.return_current_orderbook(ticker, max_levels) } -> std::same_as<std::optional<Orderbook_State>>;
    { t.get_asset_name_conversion(asset_name) } -> std::same_as<std::string>;
    { t.get_name() } -> std::same_as<std::string>;
};

int main() {
    // Static assert all the connectivity classes
    static_assert(HasRequiredMethods<Coinbase>, "Coinbase does not satisfy the required methods");

    Coinbase coinbase;

    std::unordered_map<std::string, Exchange*> exchange_map;
    std::vector<std::string> exchange_list;

    exchange_map[coinbase.get_name()] = &coinbase;
    exchange_list.push_back(coinbase.get_name());


    CoreComponent cc(std::move(exchange_map), std::move(exchange_list));
    cc.run();

    return 0;
}