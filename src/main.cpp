#include "order_book.hpp"
#include <iostream>

static void print_level(const char* label, const std::optional<LevelInfo>& lvl) {
    if (lvl) {
        std::cout << label
                  << " price=" << lvl->price_ticks
                  << " qty="   << lvl->total_qty
                  << " orders=" << lvl->order_count << '\n';
    } else {
        std::cout << label << " (empty)\n";
    }
}

int main() {
    OrderBook book;

    book.add_resting_order({1, Side::Buy,  OrderType::Limit, 10020,  5, 1});
    book.add_resting_order({2, Side::Buy,  OrderType::Limit, 10025, 10, 2});
    book.add_resting_order({3, Side::Buy,  OrderType::Limit, 10025,  5, 3});
    book.add_resting_order({4, Side::Sell, OrderType::Limit, 10030,  8, 4});
    book.add_resting_order({5, Side::Sell, OrderType::Limit, 10035,  3, 5});

    std::cout << "=== Initial book ===\n";
    print_level("Best bid:", book.best_bid());
    print_level("Best ask:", book.best_ask());

    auto snap = book.snapshot(3);
    std::cout << "\n--- Snapshot (depth 3) ---\n";
    std::cout << "Bids:\n";
    for (const auto& l : snap.bids)
        std::cout << "  " << l.price_ticks << " qty=" << l.total_qty
                  << " orders=" << l.order_count << '\n';
    std::cout << "Asks:\n";
    for (const auto& l : snap.asks)
        std::cout << "  " << l.price_ticks << " qty=" << l.total_qty
                  << " orders=" << l.order_count << '\n';

    std::cout << "\n=== After cancelling order 2 ===\n";
    book.cancel_order(2);
    print_level("Best bid:", book.best_bid());

    std::cout << "\n=== After cancelling order 3 ===\n";
    book.cancel_order(3);
    print_level("Best bid (should be 10020):", book.best_bid());

    bool ok = book.cancel_order(3);
    std::cout << "\nDouble-cancel order 3: "
              << (ok ? "WRONGLY accepted" : "correctly rejected") << '\n';
    return 0;
}
