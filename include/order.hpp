#pragma once

#include "types.hpp"

struct Order {
    OrderId   order_id    {};
    Side      side        {};
    OrderType type        {};
    Price     price_ticks {};
    Qty       qty         {};
    SeqNo     seq         {};
};
