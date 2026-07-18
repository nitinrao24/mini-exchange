#pragma once

#include "order.hpp"
#include "snapshot.hpp"

#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>

struct OrderNode {
    Order order;
};

struct PriceLevel {
    Price                 price     {};
    std::list<OrderNode>  fifo;
    Qty                   total_qty {};
};

struct OrderRef {
    Side                            side  {};
    Price                           price {};
    std::list<OrderNode>::iterator  it;
};

class OrderBook {
public:
    bool add_resting_order(const Order& order);
    bool cancel_order(OrderId id);

    std::optional<LevelInfo> best_bid() const;
    std::optional<LevelInfo> best_ask() const;
    BookSnapshot snapshot(std::size_t depth = 10) const;
    bool has_order(OrderId id) const;

    using BidMap = std::map<Price, PriceLevel, std::greater<Price>>;
    using AskMap = std::map<Price, PriceLevel>;

    BidMap& bids() { return bids_; }
    AskMap& asks() { return asks_; }

    void remove_from_index(OrderId id) { order_index_.erase(id); }
    void erase_empty_bid_level(Price p);
    void erase_empty_ask_level(Price p);

private:
    BidMap bids_;
    AskMap asks_;
    std::unordered_map<OrderId, OrderRef> order_index_;
};
