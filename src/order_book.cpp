#include "order_book.hpp"

bool OrderBook::add_resting_order(const Order& order) {
    if (order_index_.count(order.order_id)) return false;

    if (order.side == Side::Buy) {
        auto& level = bids_[order.price_ticks];
        level.price  = order.price_ticks;
        level.fifo.push_back({order});
        level.total_qty += order.qty;
        auto it = std::prev(level.fifo.end());
        order_index_[order.order_id] = {Side::Buy, order.price_ticks, it};
    } else {
        auto& level = asks_[order.price_ticks];
        level.price  = order.price_ticks;
        level.fifo.push_back({order});
        level.total_qty += order.qty;
        auto it = std::prev(level.fifo.end());
        order_index_[order.order_id] = {Side::Sell, order.price_ticks, it};
    }
    return true;
}

bool OrderBook::cancel_order(OrderId id) {
    auto idx_it = order_index_.find(id);
    if (idx_it == order_index_.end()) return false;

    const auto& ref = idx_it->second;
    if (ref.side == Side::Buy) {
        auto map_it = bids_.find(ref.price);
        if (map_it == bids_.end()) return false;
        auto& level = map_it->second;
        level.total_qty -= ref.it->order.qty;
        level.fifo.erase(ref.it);
        if (level.fifo.empty()) bids_.erase(map_it);
    } else {
        auto map_it = asks_.find(ref.price);
        if (map_it == asks_.end()) return false;
        auto& level = map_it->second;
        level.total_qty -= ref.it->order.qty;
        level.fifo.erase(ref.it);
        if (level.fifo.empty()) asks_.erase(map_it);
    }
    order_index_.erase(idx_it);
    return true;
}

std::optional<LevelInfo> OrderBook::best_bid() const {
    if (bids_.empty()) return std::nullopt;
    const auto& [price, level] = *bids_.begin();
    return LevelInfo{price, level.total_qty, level.fifo.size()};
}

std::optional<LevelInfo> OrderBook::best_ask() const {
    if (asks_.empty()) return std::nullopt;
    const auto& [price, level] = *asks_.begin();
    return LevelInfo{price, level.total_qty, level.fifo.size()};
}

BookSnapshot OrderBook::snapshot(std::size_t depth) const {
    BookSnapshot snap;
    snap.bids.reserve(depth);
    snap.asks.reserve(depth);
    std::size_t count = 0;
    for (const auto& [price, level] : bids_) {
        if (count++ >= depth) break;
        snap.bids.push_back({price, level.total_qty, level.fifo.size()});
    }
    count = 0;
    for (const auto& [price, level] : asks_) {
        if (count++ >= depth) break;
        snap.asks.push_back({price, level.total_qty, level.fifo.size()});
    }
    return snap;
}

bool OrderBook::has_order(OrderId id) const {
    return order_index_.count(id) > 0;
}

void OrderBook::erase_empty_bid_level(Price p) {
    auto it = bids_.find(p);
    if (it != bids_.end() && it->second.fifo.empty()) bids_.erase(it);
}

void OrderBook::erase_empty_ask_level(Price p) {
    auto it = asks_.find(p);
    if (it != asks_.end() && it->second.fifo.empty()) asks_.erase(it);
}
