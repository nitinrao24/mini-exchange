#include "reporter.hpp"
#include <iomanip>

Reporter::Reporter(std::ostream& out) : out_(out) {}

void Reporter::report_result(const EngineResult& result) {
    if (!result.accepted) { out_ << "  -> REJECTED: " << result.message << '\n'; return; }
    for (const auto& t : result.trades)
        out_ << "  -> TRADE  buy=" << t.buy_order_id << "  sell=" << t.sell_order_id
             << "  px=" << t.price_ticks << "  qty=" << t.qty << '\n';
    print_bbo(result.best_bid, result.best_ask);
}

void Reporter::report_snapshot(const BookSnapshot& snap, std::size_t) {
    out_ << "  Bids:\n";
    if (snap.bids.empty()) { out_ << "    (empty)\n"; }
    else for (const auto& l : snap.bids)
        out_ << "    " << std::setw(8) << l.price_ticks
             << "   qty=" << std::setw(5) << l.total_qty
             << "   orders=" << l.order_count << '\n';
    out_ << "  Asks:\n";
    if (snap.asks.empty()) { out_ << "    (empty)\n"; }
    else for (const auto& l : snap.asks)
        out_ << "    " << std::setw(8) << l.price_ticks
             << "   qty=" << std::setw(5) << l.total_qty
             << "   orders=" << l.order_count << '\n';
}

void Reporter::report_top_of_book(const std::optional<LevelInfo>& bid,
                                   const std::optional<LevelInfo>& ask) {
    print_bbo(bid, ask);
}

void Reporter::print_bbo(const std::optional<LevelInfo>& bid,
                          const std::optional<LevelInfo>& ask) {
    out_ << "  -> ";
    if (bid) out_ << "bid=" << bid->price_ticks << "x" << bid->total_qty;
    else      out_ << "bid=---";
    out_ << "   ";
    if (ask) out_ << "ask=" << ask->price_ticks << "x" << ask->total_qty;
    else      out_ << "ask=---";
    out_ << '\n';
}
