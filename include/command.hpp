#pragma once

#include "order.hpp"
#include "types.hpp"
#include <optional>

struct Command {
    CommandType            type{};
    std::optional<Order>   order;
    std::optional<OrderId> cancel_id;
};
