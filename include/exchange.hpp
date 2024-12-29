#pragma once

#ifndef EXCHANGE_HPP
#define EXCHANGE_HPP

#include <string>

class Exchange {
public:
    virtual ~Exchange() = default;

    virtual std::string get_name() = 0;
};

#endif // EXCHANGE_HPP