#pragma once

#include <cstdint>

using OrderId = std::uint64_t;
using TradeId = std::uint64_t;
using Price   = std::int64_t;
using Qty     = std::uint32_t;
using SeqNo   = std::uint64_t;

enum class Side        { Buy, Sell };
enum class OrderType   { Limit, Market };
enum class CommandType { Place, Cancel, Snapshot, TopOfBook };

enum class OrderStatus {
    Accepted,
    PartiallyFilled,
    Filled,
    Cancelled,
    Rejected
};
