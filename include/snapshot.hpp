#pragma once

#include "types.hpp"
#include <cstddef>
#include <vector>

struct LevelInfo {
    Price       price_ticks  {};
    Qty         total_qty    {};
    std::size_t order_count  {};
};

struct BookSnapshot {
    std::vector<LevelInfo> bids;
    std::vector<LevelInfo> asks;
};
