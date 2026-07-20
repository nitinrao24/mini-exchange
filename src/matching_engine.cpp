#include "matching_engine.hpp"
#include <algorithm>

EngineResult MatchingEngine::process(const Command& cmd) {
    switch (cmd.type) {
        case CommandType::Place: {
            if (!cmd.order) return {false, "Place command missing order"};
            Order o = *cmd.order;
            o.seq   = next_seq_++;
            return process_place(o);
        }
        case CommandType::Cancel: {
            if (!cmd.cancel_id) return {false, "Cancel command missing id"};
            bool ok = cancel(*cmd.cancel_id);
            EngineResult r;
            r.accepted = ok;
            r.message  = ok ? "" : "Order not found";
            r.best_bid = book_.best_bid();
            r.best_ask = book_.best_ask();
            return r;
        }
        case CommandType::Snapshot:
        case CommandType::TopOfBook: {
            EngineResult r;
            r.best_bid = book_.best_bid();
            r.best_ask = book_.best_ask();
            return r;
        }
    }
    return {false, "Unknown command type"};
}

EngineResult MatchingEngine::submit_limit(OrderId id, Side side, Price price, Qty qty) {
    Order o{id, side, OrderType::Limit, price, qty, next_seq_++};
    return process_place(o);
}

EngineResult MatchingEngine::submit_market(OrderId id, Side side, Qty qty) {
    Order o{id, side, OrderType::Market, 0, qty, next_seq_++};
    return process_place(o);
}

bool MatchingEngine::cancel(OrderId id) { return book_.cancel_order(id); }

std::optional<LevelInfo> MatchingEngine::best_bid() const { return book_.best_bid(); }
std::optional<LevelInfo> MatchingEngine::best_ask() const { return book_.best_ask(); }
BookSnapshot MatchingEngine::snapshot(std::size_t depth) const { return book_.snapshot(depth); }

EngineResult MatchingEngine::process_place(Order incoming) {
    EngineResult result;
    if (incoming.side == Side::Buy)  match_buy (incoming, result);
    else                             match_sell(incoming, result);
    if (incoming.qty > 0 && incoming.type == OrderType::Limit)
        book_.add_resting_order(incoming);
    result.best_bid = book_.best_bid();
    result.best_ask = book_.best_ask();
    return result;
}

void MatchingEngine::match_buy(Order& incoming, EngineResult& result) {
    auto& asks = book_.asks();
    while (incoming.qty > 0 && !asks.empty()) {
        auto        level_it  = asks.begin();
        Price       ask_price = level_it->first;
        PriceLevel& level     = level_it->second;
        if (incoming.type == OrderType::Limit && incoming.price_ticks < ask_price) break;
        Order& resting = level.fifo.front().order;
        Qty    matched = std::min(incoming.qty, resting.qty);
        result.trades.push_back(Trade{next_trade_id_++, incoming.order_id,
                                      resting.order_id, ask_price, matched, next_seq_++});
        incoming.qty    -= matched;
        resting.qty     -= matched;
        level.total_qty -= matched;
        if (resting.qty == 0) {
            book_.remove_from_index(resting.order_id);
            level.fifo.pop_front();
            if (level.fifo.empty()) asks.erase(level_it);
        }
    }
}

void MatchingEngine::match_sell(Order& incoming, EngineResult& result) {
    auto& bids = book_.bids();
    while (incoming.qty > 0 && !bids.empty()) {
        auto        level_it  = bids.begin();
        Price       bid_price = level_it->first;
        PriceLevel& level     = level_it->second;
        if (incoming.type == OrderType::Limit && incoming.price_ticks > bid_price) break;
        Order& resting = level.fifo.front().order;
        Qty    matched = std::min(incoming.qty, resting.qty);
        result.trades.push_back(Trade{next_trade_id_++, resting.order_id,
                                      incoming.order_id, bid_price, matched, next_seq_++});
        incoming.qty    -= matched;
        resting.qty     -= matched;
        level.total_qty -= matched;
        if (resting.qty == 0) {
            book_.remove_from_index(resting.order_id);
            level.fifo.pop_front();
            if (level.fifo.empty()) bids.erase(level_it);
        }
    }
}
