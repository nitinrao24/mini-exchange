#include "matching_engine.hpp"
#include <iostream>

static void print_trade(const Trade& t) {
    std::cout << "  TRADE  buy=" << t.buy_order_id << " sell=" << t.sell_order_id
              << " px=" << t.price_ticks << " qty=" << t.qty << '\n';
}

static void print_bbo(const EngineResult& r) {
    std::cout << "  BBO  bid=";
    if (r.best_bid) std::cout << r.best_bid->price_ticks << "x" << r.best_bid->total_qty;
    else             std::cout << "---";
    std::cout << "  ask=";
    if (r.best_ask) std::cout << r.best_ask->price_ticks << "x" << r.best_ask->total_qty;
    else             std::cout << "---";
    std::cout << '\n';
}

int main() {
    MatchingEngine engine;

    engine.submit_limit(1, Side::Buy,  10020, 10);
    engine.submit_limit(2, Side::Buy,  10025,  5);
    engine.submit_limit(3, Side::Sell, 10030,  8);
    engine.submit_limit(4, Side::Sell, 10035,  3);

    std::cout << "--- Limit buy 5 @ 10030 ---\n";
    { auto r = engine.submit_limit(5, Side::Buy, 10030, 5);
      for (const auto& t : r.trades) print_trade(t); print_bbo(r); }

    std::cout << "\n--- FIFO: two resting sells at 10030 ---\n";
    engine.submit_limit(6, Side::Sell, 10030, 4);
    engine.submit_limit(7, Side::Sell, 10030, 4);
    { auto r = engine.submit_limit(8, Side::Buy, 10030, 4);
      for (const auto& t : r.trades) print_trade(t); print_bbo(r); }

    std::cout << "\n--- Market sell qty=12 ---\n";
    { auto r = engine.submit_market(9, Side::Sell, 12);
      for (const auto& t : r.trades) print_trade(t); print_bbo(r); }

    std::cout << "\n--- Cancel order 1 ---\n";
    std::cout << "  cancel(1) -> " << (engine.cancel(1) ? "ok" : "not found") << '\n';
    std::cout << "  cancel(1) -> " << (engine.cancel(1) ? "ok" : "not found (correct)") << '\n';
    return 0;
}
