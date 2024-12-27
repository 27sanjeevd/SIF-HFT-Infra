#include "include/coinbase.hpp"

#include <iostream>
#include <type_traits>
#include <concepts>

template <typename T>
concept HasRequiredMethods = requires(T t, const std::string& url) {
    { t.return_request(url) } -> std::same_as<std::optional<std::string>>;
};


int main() {
    static_assert(HasRequiredMethods<Coinbase>, "Coinbase does not satisfy the required methods");

    std::cout << "HI\n";

    return 0;
}