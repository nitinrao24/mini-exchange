#pragma once

#include "command.hpp"
#include "order_book.hpp"
#include "trade.hpp"

#include <optional>
#include <string>
#include <vector>

struct EngineResult {
    bool                     accepted{true};
    std::string              message{};
    std::vector<Trade>       trades{};
    std::optional<LevelInfo> best_bid{};
    std::optional<LevelInfo> best_ask{};
};

class MatchingEngine {
public:
    EngineResult process(const Command& cmd);
    EngineResult submit_limit(OrderId id, Side side, Price price, Qty qty);
    EngineResult submit_market(OrderId id, Side side, Qty qty);
    bool         cancel(OrderId id);

    std::optional<LevelInfo> best_bid() const;
    std::optional<LevelInfo> best_ask() const;
    BookSnapshot             snapshot(std::size_t depth = 10) const;

private:
    OrderBook book_;
    TradeId   next_trade_id_{1};
    SeqNo     next_seq_{1};

    EngineResult process_place(Order incoming);
    void         match_buy (Order& incoming, EngineResult& result);
    void         match_sell(Order& incoming, EngineResult& result);
};
